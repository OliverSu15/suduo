#include "TcpConnection.h"

#include <cerrno>

#include "suduo/base/Logger.h"
#include "suduo/base/WeakCallback.h"
#include "suduo/net/Callbacks.h"
#include "suduo/net/Channel.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/Socket.h"
#include "suduo/net/SocketOpt.h"

using TcpConnection = suduo::net::TcpConnection;
using namespace suduo::net;
using namespace suduo;

void suduo::net::defaultConnectionCallback(const TcpConnectionPtr& conn) {
  LOG_TRACE << conn->local_address().to_Ip_port() << " -> "
            << conn->peer_address().to_Ip_port() << " is "
            << (conn->connected() ? "UP" : "DOWN");
  // do not call conn->forceClose(), because some users want to register message
  // callback only.
}

void suduo::net::defaultMessageCallback(const TcpConnectionPtr& conn,
                                        Buffer* buffer, Timestamp receiveTime) {
  buffer->retrieve_all();
}

TcpConnection::TcpConnection(EventLoop* loop, const std::string& name,
                             int sock_fd, const InetAddress& local_addr,
                             const InetAddress& peer_addr)
    : _loop(loop),
      _name(name),
      _state(Connecting),
      _reading(true),
      _socket(new Socket(sock_fd)),
      _channel(new Channel(loop, sock_fd)),
      _local_addr(local_addr),
      _peer_addr(peer_addr),
      _hight_water_mark(64 * 1024 * 1024) {
  _channel->set_read_callback(
      std::bind(&TcpConnection::handle_read, this, std::placeholders::_1));
  _channel->set_write_callback(std::bind(&TcpConnection::handle_write, this));
  _channel->set_close_callback(std::bind(&TcpConnection::handle_close, this));
  _channel->set_error_callback(std::bind(&TcpConnection::handle_error, this));
  LOG_DEBUG << "TcpConnection::ctor[" << _name << "] at " << this
            << " fd=" << sock_fd;
  _socket->set_keep_alive(true);
}

TcpConnection::~TcpConnection() {
  LOG_DEBUG << "TcpConnection::dtor[" << _name << "] at " << this
            << " fd=" << _channel->fd() << " state=" << stateToString();
  assert(_state == Disconnected);
}

bool TcpConnection::get_tcp_info(tcp_info* info) const {
  return _socket->get_Tcp_info(info);
}

std::string TcpConnection::get_tcp_info_string() const {
  char buf[1024];
  buf[0] = '\0';
  _socket->get_Tcp_info_string(buf, sizeof(buf));
  return buf;
}

void TcpConnection::send(const void* date, int len) {
  send(static_cast<const char*>(date), len);
}

void TcpConnection::send(const std::string& message) {
  if (_state == Connected) {
    if (_loop->is_in_loop_thread()) {
      send_in_loop(message);
    } else {
      void (TcpConnection::*fp)(const std::string& message) =
          &TcpConnection::send_in_loop;
      _loop->run_in_loop(std::bind(fp, this, message));
    }
  }
}

// FIXME efficiency!!!
void TcpConnection::send(Buffer* buf) {
  if (_state == Connected) {
    if (_loop->is_in_loop_thread()) {
      send_in_loop(buf->peek(), buf->readable_bytes());
      buf->retrieve_all();
    } else {
      void (TcpConnection::*fp)(const std::string& message) =
          &TcpConnection::send_in_loop;
      _loop->run_in_loop(std::bind(fp, this, buf->retrieve_all_as_string()));
    }
  }
}

void TcpConnection::send_in_loop(const std::string& message) {
  send_in_loop(message.data(), message.size());
}

void TcpConnection::send_in_loop(const void* message, size_t len) {
  _loop->assert_in_loop_thread();
  ssize_t nwrote = 0;
  size_t remaining = len;
  bool fault_error = false;
  if (_state == Disconnected) {
    LOG_WARN << "disconnected, give up writing";
    return;
  }
  if (!_channel->is_writing() && _output_buffer.readable_bytes() == 0) {
    nwrote = sockets::write(_channel->fd(), message, len);
    if (nwrote >= 0) {
      remaining = len - nwrote;
      if (remaining == 0 && _write_complete_callback) {
        _loop->queue_in_loop(
            std::bind(_write_complete_callback, shared_from_this()));
      }
    } else {
      nwrote = 0;
      if (errno != EWOULDBLOCK) {
        LOG_SYSERR << "TcpConnection::sendInLoop";
        if (errno == EPIPE || errno == ECONNRESET)  // FIXME: any others?
        {
          fault_error = true;
        }
      }
    }
  }
  assert(remaining <= len);
  if (!fault_error && remaining > 0) {
    size_t old_len = _output_buffer.readable_bytes();
    if (old_len + remaining >= _hight_water_mark &&
        old_len < _hight_water_mark && _high_water_mark_callback) {
      _loop->queue_in_loop(std::bind(_high_water_mark_callback,
                                     shared_from_this(), old_len + remaining));
    }
    _output_buffer.append(static_cast<const char*>(message) + nwrote,
                          remaining);
    if (!_channel->is_writing()) {
      _channel->enable_writing();
    }
  }
}

void TcpConnection::shutdown() {
  if (_state == Connected) {
    set_state(Disconnecting);
    _loop->run_in_loop(std::bind(&TcpConnection::shutdown_in_loop, this));
  }
}

void TcpConnection::shutdown_in_loop() {
  _loop->assert_in_loop_thread();
  if (!_channel->is_writing()) {
    // we are not writing
    _socket->shutdown_write();
  }
}

void TcpConnection::force_close() {
  if (_state == Connected || _state == Disconnecting) {
    set_state(Disconnecting);
    _loop->queue_in_loop(
        std::bind(&TcpConnection::force_close_in_loop, shared_from_this()));
  }
}

void TcpConnection::force_close_with_delay(double seconds) {
  if (_state == Connected || _state == Disconnecting) {
    set_state(Disconnecting);
    _loop->run_after(seconds, makeWeakCallback(shared_from_this(),
                                               &TcpConnection::force_close));
  }
}

void TcpConnection::force_close_in_loop() {
  _loop->assert_in_loop_thread();
  if (_state == Connected || _state == Disconnecting) {
    // as if we received 0 byte in handleRead();
    handle_close();
  }
}

const char* TcpConnection::stateToString() const {
  switch (_state) {
    case Disconnected:
      return "Disconnected";
    case Connecting:
      return "Connecting";
    case Connected:
      return "Connected";
    case Disconnecting:
      return "Disconnecting";
    default:
      return "unknown state";
  }
}

void TcpConnection::set_tcp_no_delay(bool on) { _socket->set_Tcp_no_delay(on); }

void TcpConnection::start_read() {
  _loop->run_in_loop(std::bind(&TcpConnection::start_read_in_loop, this));
}

void TcpConnection::start_read_in_loop() {
  _loop->assert_in_loop_thread();
  if (!_reading || !_channel->is_reading()) {
    _channel->enable_reading();
    _reading = true;
  }
}

void TcpConnection::stop_read() {
  _loop->run_in_loop(std::bind(&TcpConnection::stop_read_in_loop, this));
}
void TcpConnection::stop_read_in_loop() {
  _loop->assert_in_loop_thread();
  if (_reading || _channel->is_reading()) {
    _channel->disable_reading();
    _reading = false;
  }
}

void TcpConnection::connect_established() {
  _loop->assert_in_loop_thread();
  assert(_state == Connecting);
  set_state(Connected);
  _channel->tie(shared_from_this());
  _channel->enable_reading();

  _connection_callback(shared_from_this());
}

void TcpConnection::connect_destroyed() {
  _loop->assert_in_loop_thread();
  if (_state == Connected) {
    set_state(Disconnected);
    _channel->disable_all();
    _connection_callback(shared_from_this());
  }
  _channel->remove();
}

void TcpConnection::handle_read(Timestamp receive_time) {
  _loop->assert_in_loop_thread();
  int saved_errno = 0;
  ssize_t n = _input_buffer.read_fd(_channel->fd(), &saved_errno);
  if (n > 0) {
    _message_callback(shared_from_this(), &_input_buffer, receive_time);
  } else if (n == 0) {
    handle_close();
  } else {
    errno = saved_errno;
    LOG_SYSERR << "TcpConnection::handleRead";
    handle_error();
  }
}

void TcpConnection::handle_write() {
  _loop->assert_in_loop_thread();
  if (_channel->is_writing()) {
    ssize_t n = sockets::write(_channel->fd(), _output_buffer.peek(),
                               _output_buffer.readable_bytes());
    if (n > 0) {
      _output_buffer.retrieve(n);
      if (_output_buffer.readable_bytes() == 0) {
        _channel->disable_writing();
        if (_write_complete_callback) {
          _loop->queue_in_loop(
              std::bind(_write_complete_callback, shared_from_this()));
        }
        if (_state == Disconnecting) {
          shutdown_in_loop();
        }
      }
    } else {
      LOG_SYSERR << "TcpConnection::handleWrite";
    }
  } else {
    LOG_TRACE << "Connection fd = " << _channel->fd()
              << " is down, no more writing";
  }
}

void TcpConnection::handle_close() {
  _loop->assert_in_loop_thread();
  LOG_TRACE << "fd = " << _channel->fd() << " state = " << stateToString();
  assert(_state == Connected || _state == Disconnecting);

  set_state(Disconnected);
  _channel->disable_all();

  TcpConnectionPtr guard_this(shared_from_this());
  _connection_callback(guard_this);
  _close_callback(guard_this);
}

void TcpConnection::handle_error() {
  int err = sockets::get_socket_error(_channel->fd());
  // TODO fix it
  //  LOG_ERROR << "TcpConnection::handleError [" << _name
  //            << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}