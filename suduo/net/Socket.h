#ifndef SUDUO_SOCKET_H
#define SUDUO_SOCKET_H

#include <utility>

#include "suduo/base/noncopyable.h"
namespace suduo {
namespace net {
class Socket : noncopyable {
 public:
  explicit Socket(int sock_fd) : _sock_fd(sock_fd) {}
  explicit Socket(Socket&& socket) : _sock_fd(socket._sock_fd) {}
  ~Socket();

  int fd() const { return _sock_fd; }

 private:
  const int _sock_fd;
};
}  // namespace net
}  // namespace suduo

#endif