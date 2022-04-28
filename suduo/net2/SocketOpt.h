#ifndef SOCKET_OPT_H
#define SOCKET_OPT_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>

#include <cstddef>

#include "suduo/base/Logger.h"
namespace suduo {
namespace net {
namespace sockets {

template <typename ToPointer, typename FromPointer>
ToPointer* pointer_cast(FromPointer* ptr) {
  return static_cast<ToPointer*>(static_cast<void*>(ptr));
}
template <typename ToPointer, typename FromPointer>
const ToPointer* pointer_cast_const(const FromPointer* ptr) {
  return static_cast<const ToPointer*>(static_cast<const void*>(ptr));
}

// make the biggest len as the len for every one
static const socklen_t addr_len = sizeof(sockaddr_in6);

inline int create_nonblocking_or_abort(sa_family_t family) {
  int sockfd = socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);

  if (sockfd < 0) {
    LOG_SYSFATAL << "sockets::create_Nonblocking:failed";
  }

  return sockfd;
}

inline int connect(int sockfd, const sockaddr* addr) {
  return connect(sockfd, addr, sockets::addr_len);
}

inline void bind_or_abort(int sockfd, const sockaddr* addr) {
  if (bind(sockfd, addr, sockets::addr_len) < 0) {
    LOG_SYSFATAL << "sockets::bind_or_abort";
  }
}

inline void listen_or_abort(int sockfd) {
  if (listen(sockfd, SOMAXCONN) < 0) {
    LOG_SYSFATAL << "sockets::listen_or_abort";
  }
}

inline void close(int sockfd) {
  if (::close(sockfd) < 0) {
    LOG_SYSFATAL << "sockets::close";
  }
}

inline void shutdown_write(int sockfd) {
  if (shutdown(sockfd, SHUT_WR) < 0) {
    LOG_SYSFATAL << "sockets::shutdown_write";
  }
}

int accept(int sockfd, sockaddr_in6* addr);

inline ssize_t read(int sockfd, void* buf, size_t count) {
  return ::read(sockfd, buf, count);
}
inline ssize_t readv(int sockfd, const iovec* iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}
inline ssize_t write(int sockfd, const void* buf, size_t count) {
  return ::write(sockfd, buf, count);
}

void to_Ip_port(char* buf, size_t size, const sockaddr* addr);
void to_Ip(char* buf, size_t size, const sockaddr* addr);

void from_Ip_port(const char* ip, uint16_t port, sockaddr_in* addr);
void from_Ip_port(const char* ip, uint16_t port, sockaddr_in6* addr);

int get_socket_error(int sockfd);

inline const sockaddr* sockaddr_cast(const sockaddr_in* addr) {
  return pointer_cast_const<sockaddr, sockaddr_in>(addr);
}
inline const sockaddr* sockaddr_cast(const sockaddr_in6* addr) {
  return pointer_cast_const<sockaddr, sockaddr_in6>(addr);
}
inline sockaddr* sockaddr_cast(sockaddr_in6* addr) {
  return pointer_cast<sockaddr, sockaddr_in6>(addr);
}
inline const sockaddr_in* sockaddr_in_cast(const sockaddr* addr) {
  return pointer_cast_const<sockaddr_in, sockaddr>(addr);
}
inline const sockaddr_in6* sockaddr_in6_cast(const sockaddr* addr) {
  return pointer_cast_const<sockaddr_in6, sockaddr>(addr);
}

sockaddr_in6 get_local_addr(int sockfd);
sockaddr_in6 get_peer_addr(int sockfd);
bool is_self_connect(int sockfd);

}  // namespace sockets
}  // namespace net
}  // namespace suduo
#endif