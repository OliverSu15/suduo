#include "suduo/net/Connector.h"

#include "suduo/base/Logger.h"
#include "suduo/net/Channel.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/SocketOpt.h"

using Connector = suduo::net::Connector;

const int Connector::MAX_RETRY_DELAY_MS = 30 * 1000;
const int Connector::INIT_RETRY_DELAY_MS = 500;

Connector::Connector(EventLoop* loop, const InetAddress& server_addr)
    : _loop(loop),
      _server_addr(server_addr),
      _connect(false),
      _state(Disconnected),
      retry_delay_ms(INIT_RETRY_DELAY_MS) {
  LOG_DEBUG << "ctor[" << this << "]";
}

Connector::~Connector() {
  LOG_DEBUG << "dtor[" << this << "]";
  assert(!_channel);
}

void Connector::start() {
  _connect = true;
  _loop->run_in_loop(std::bind(&Connector::start_in_loop, this));
}

void Connector::start_in_loop() {
  _loop->assert_in_loop_thread();
  assert(_state == Disconnected);
  if (_connect) {
    connect();
  } else {
    LOG_DEBUG << "do not connect";
  }
}

void Connector::stop() {
  _connect = false;
  _loop->queue_in_loop(
      std::bind(&Connector::stop_in_loop, this));  // FIXME: unsafe
                                                   // FIXME: cancel timer
}

void Connector::stop_in_loop() {
  _loop->assert_in_loop_thread();
  if (_state == Connecting) {
    set_state(Disconnected);
    int sock_fd = remove_and_reset_channel();
    retry(sock_fd);
  }
}

void Connector::connect() {
  int sock_fd = sockets::create_nonblocking_or_abort(_server_addr.family());
  int ret = sockets::connect(sock_fd, _server_addr.get_sock_addr());
  int saved_errno = (ret == 0) ? 0 : errno;
  switch (saved_errno) {
    // TODO error handle
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
      connecting(sock_fd);
      break;
    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
      retry(sock_fd);
      break;
    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
      LOG_SYSERR << "connect error in Connector::startInLoop " << saved_errno;
      sockets::close(sock_fd);
      break;
    default:
      LOG_SYSERR << "Unexpected error in Connector::startInLoop "
                 << saved_errno;
      sockets::close(sock_fd);
      break;
  }
}

void Connector::restart() {
  _loop->assert_in_loop_thread();
  set_state(Disconnected);
  retry_delay_ms = INIT_RETRY_DELAY_MS;
  _connect = true;
  start_in_loop();
}

void Connector::connecting(int sockfd) {
  set_state(Connecting);
  assert(!_channel);
  _channel.reset(new Channel(_loop->poller(), sockfd));
  _channel->set_write_callback(std::bind(&Connector::handle_write, this));
  _channel->set_error_callback(std::bind(&Connector::handle_error, this));

  _channel->enable_writing();
}

int Connector::remove_and_reset_channel() {
  _channel->disable_all();
  _channel->remove();
  int sock_fd = _channel->fd();
  _loop->queue_in_loop(std::bind(&Connector::reset_channel, this));
  return sock_fd;
}

void Connector::reset_channel() { _channel.reset(); }

void Connector::handle_write() {
  LOG_TRACE << "Connector::handleWrite " << _state;
  if (_state == Connecting) {
    int sock_fd = remove_and_reset_channel();
    int err = sockets::get_socket_error(sock_fd);
    if (err) {
      LOG_WARN << "Connector::handleWrite - SO_ERROR = " << err << " "
               << strerror_tl(err);
      retry(sock_fd);
    } else if (sockets::is_self_connect(sock_fd)) {
      LOG_WARN << "Connector::handleWrite - Self connect";
      retry(sock_fd);
    } else {
      set_state(Connected);
      if (_connect) {
        _new_connection_callback(sock_fd);
      } else {
        sockets::close(sock_fd);
      }
    }
  } else {
    assert(_state == Disconnected);
  }
}

void Connector::handle_error() {
  LOG_ERROR << "Connector::handleError state=" << _state;
  if (_state == Connecting) {
    int sock_fd = remove_and_reset_channel();
    int err = sockets::get_socket_error(sock_fd);
    LOG_TRACE << "SO_ERROR = " << err << " " << strerror_tl(err);
    retry(sock_fd);
  }
}

void Connector::retry(int sockfd) {
  sockets::close(sockfd);
  set_state(Disconnected);
  if (_connect) {
    LOG_INFO << "Connector::retry - Retry connecting to "
             << _server_addr.to_Ip_port() << " in " << retry_delay_ms
             << " milliseconds. ";
    _loop->run_after(retry_delay_ms / 1000.0,
                     std::bind(&Connector::start_in_loop, shared_from_this()));
    retry_delay_ms = std::min(retry_delay_ms * 2, MAX_RETRY_DELAY_MS);
  } else {
    LOG_DEBUG << "do not connect";
  }
}