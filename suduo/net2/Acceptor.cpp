#include "suduo/net2/Acceptor.h"

#include <asm-generic/errno-base.h>
#include <errno.h>
#include <fcntl.h>

#include "suduo/base/Logger.h"
#include "suduo/net2/EventLoop.h"
#include "suduo/net2/InetAddress.h"
#include "suduo/net2/SocketOpt.h"

using Acceptor = suduo::net::Acceptor;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listen_addr,
                   bool reuse_port)
    : _loop(loop),
      _accept_socket(
          sockets::create_nonblocking_or_abort(listen_addr.family())),
      _accept_channel(loop->poller(), _accept_socket.fd()),
      _listening(false),
      _idle_fd(open("/dev/null", O_RDONLY | O_CLOEXEC)) {
  _accept_socket.set_reuse_addr(true);
  _accept_socket.set_reuse_port(reuse_port);
  _accept_socket.bind(listen_addr);
  _accept_channel.set_read_callback(std::bind(&Acceptor::handle_read, this));
}

Acceptor::~Acceptor() {
  _accept_channel.disable_all();
  _accept_channel.remove();
}

void Acceptor::listen() {
  _loop->assert_in_loop_thread();
  _listening = true;
  _accept_socket.listen();
  _accept_channel.enable_reading();
}

void Acceptor::handle_read() {
  _loop->assert_in_loop_thread();
  InetAddress peer_addr;
  int connfd = _accept_socket.accept(&peer_addr);
  if (connfd >= 0) {
    if (_callback) {
      _callback(connfd, peer_addr);
    } else {
      sockets::close(connfd);
    }
  } else {
    LOG_SYSERR << "in Acceptor::handleRead";
    // Read the section named "The special problem of
    // accept()ing when you can't" in libev's doc.
    // By Marc Lehmann, author of libev.
    if (errno == EMFILE || errno == ENFILE) {
      ::close(_idle_fd);
      _idle_fd = ::accept(_accept_socket.fd(), NULL, NULL);
      ::close(_idle_fd);
      _idle_fd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
    }
  }
}