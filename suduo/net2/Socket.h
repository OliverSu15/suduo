#ifndef SUDUO_SOCKET_H
#define SUDUO_SOCKET_H
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "suduo/base/Logger.h"
#include "suduo/base/noncopyable.h"
#include "suduo/net2/InetAddress.h"
namespace suduo {
namespace net {
class Socket : noncopyable {
 public:
  explicit Socket(sa_family_t family)
      : _sock_fd(sockets::create_nonblocking_or_abort(family)) {}
  explicit Socket(int sock_fd) : _sock_fd(sock_fd) {}
  ~Socket() { sockets::close(_sock_fd); }

  int fd() const { return _sock_fd; }

  bool get_TCP_info(tcp_info* info) const;
  // test usage only
  bool get_TCP_info_string(char* buf, int len) const;

  void bind(const InetAddress& local_addr) const {
    sockets::bind_or_abort(_sock_fd, local_addr.get_sock_addr());
  }
  void listen() const { sockets::listen_or_abort(_sock_fd); }
  int accept(InetAddress* peer_addr) const {
    int conn_fd = -1;
    if (peer_addr->is_ipv6()) {
      sockaddr_in6 addr;
      conn_fd = sockets::accept(_sock_fd, &addr);
      if (conn_fd >= 0) {
        peer_addr->set_sock_addr(addr);
      }
    } else {
      sockaddr_in addr;
      conn_fd = sockets::accept(_sock_fd, &addr);
      if (conn_fd >= 0) {
        peer_addr->set_sock_addr(addr);
      }
    }
    return conn_fd;
  }

  int set_sock_opt(int level, int opt_name, const void* val, socklen_t len) {
    return ::setsockopt(_sock_fd, level, opt_name, val, len);
  }

  void set_TCP_no_delay(bool on) {
    int opt_val = on ? 1 : 0;
    if (set_sock_opt(IPPROTO_TCP, TCP_NODELAY, &opt_val, sizeof(opt_val)) !=
        0) {
      LOG_ERROR << "Socket::set_TCP_no_delay()";
    }
  }
  void set_reuse_addr(bool on) {
    int opt_val = on ? 1 : 0;
    if (set_sock_opt(SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val)) !=
        0) {
      LOG_ERROR << "Socket::set_reuse_addr()";
    }
  }
  void set_reuse_port(bool on) {
    int opt_val = on ? 1 : 0;
    if (set_sock_opt(SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(opt_val)) !=
        0) {
      LOG_ERROR << "Socket::set_reuse_port()";
    }
  }
  void set_keep_alive(bool on) {
    int opt_val = on ? 1 : 0;
    if (set_sock_opt(SOL_SOCKET, SO_KEEPALIVE, &opt_val, sizeof(opt_val)) !=
        0) {
      LOG_ERROR << "Socket::set_keep_alive()";
    }
  }

 private:
  const int _sock_fd;
};
}  // namespace net
}  // namespace suduo
#endif
