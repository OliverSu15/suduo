#ifndef SUDUO_SOCKET_H
#define SUDUO_SOCKET_H

#include <netinet/tcp.h>

#include <utility>

#include "suduo/base/noncopyable.h"
namespace suduo {
namespace net {
class InetAddress;
class Socket : noncopyable {
 public:
  explicit Socket(int sock_fd) : _sock_fd(sock_fd) {}
  explicit Socket(Socket&& socket) : _sock_fd(socket._sock_fd) {}
  ~Socket();

  int fd() const { return _sock_fd; }

  bool get_Tcp_info(tcp_info* info) const;
  bool get_Tcp_info_string(char* buf, int len) const;

  void bind_address(const InetAddress& local_addr);
  void listen();
  int accept(InetAddress* peer_addr);

  void shutdown_write();

  void set_Tcp_no_delay(bool on);
  void set_reuse_addr(bool on);
  void set_reuse_port(bool on);
  void set_keep_alive(bool on);

 private:
  const int _sock_fd;
};
}  // namespace net
}  // namespace suduo

#endif