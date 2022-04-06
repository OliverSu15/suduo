#include "SocketOpt.h"

#include <arpa/inet.h>
#include <asm-generic/socket.h>
#include <bits/types/cookie_io_functions_t.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include "suduo/base/Logger.h"
#include "suduo/net/Endian.h"
// using namespace suduo::net::socket;
using namespace suduo::net;

// TODO find a better way to do it
const sockaddr* sockets::sockaddr_cast(const sockaddr_in* addr) {
  // TODO may have a better way to do it
  return static_cast<const sockaddr*>(static_cast<const void*>(addr));
}
const sockaddr* sockets::sockaddr_cast(const sockaddr_in6* addr) {
  // TODO may have a better way to do it
  return static_cast<const sockaddr*>(static_cast<const void*>(addr));
}
sockaddr* sockets::sockaddr_cast(sockaddr_in6* addr) {
  return static_cast<sockaddr*>(static_cast<void*>(addr));
}
const sockaddr_in* sockaddr_in_cast(const sockaddr* addr) {
  return static_cast<const sockaddr_in*>(static_cast<const void*>(addr));
}
const sockaddr_in6* sockaddr_in6_cast(const sockaddr* addr) {
  return static_cast<const sockaddr_in6*>(static_cast<const void*>(addr));
}

int sockets::create_Nonblocking_or_abort(sa_family_t family) {
  int sockfd =
      socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
  if (sockfd < 0) {
    // TODO make a  Log which can abort
    LOG_FATAL << "sockets::create_Nonblocking";
  }
  return sockfd;
}

int sockets::connect(int sockfd, const sockaddr* addr) {
  // TODO not sure is going to work
  return connect(sockfd, addr, sockets::addr_len);
}

void sockets::bind_or_abort(int sockfd, const sockaddr* addr) {
  // int ret = bind(sockfd,addr,sizeof(*addr));
  if (bind(sockfd, addr, sockets::addr_len) < 0) {
    // TODO make a  Log which can abort
    LOG_FATAL << "sockets::bind_or_abort";
  }
}

void sockets::listen_or_abort(int sockfd) {
  // int ret = listen(sockfd, SOMAXCONN);
  if (listen(sockfd, SOMAXCONN) < 0) {
    // TODO make a  Log which can abort
    LOG_FATAL << "sockets::listen_or_abort";
  }
}

int sockets::accept(int sockfd, sockaddr_in6* addr) {
  socklen_t len = addr_len;
  int connection_fd =
      accept4(sockfd, sockaddr_cast(addr), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connection_fd < 0) {
    // TODO handle error
  }
  return connection_fd;
}

ssize_t sockets::read(int sockfd, void* buf, size_t count) {
  return ::read(sockfd, buf, count);
}
ssize_t sockets::readv(int sockfd, const iovec* iov, int iovcnt) {
  return ::readv(sockfd, iov, iovcnt);
}
ssize_t sockets::write(int sockfd, const void* buf, size_t count) {
  return ::write(sockfd, buf, count);
}

void sockets::close(int sockfd) {
  if (::close(sockfd) < 0) {
    // TODO handle error
  }
}
void sockets::shutdown_write(int sockfd) {
  if (shutdown(sockfd, SHUT_WR) < 0) {
    // TODO handle error
  }
}

// TODO change it to other style
void sockets::to_Ip_port(char* buf, size_t size, const sockaddr* addr) {
  if (addr->sa_family == AF_INET6) {
    buf[0] = '[';
    to_Ip(buf + 1, size - 1, addr);
    size_t end = strlen(buf);
    u_int16_t port = sockets::network_to_host_16(
        sockets::sockaddr_in6_cast(addr)->sin6_port);
    snprintf(buf + end, size - end, "]:%u", port);
  } else if (addr->sa_family == AF_INET) {
    to_Ip(buf, size, addr);
    size_t end = strlen(buf);
    uint16_t port =
        sockets::network_to_host_16(sockets::sockaddr_in_cast(addr)->sin_port);
    snprintf(buf + end, size - end, ":%u", port);
  }
}
void sockets::to_Ip(char* buf, size_t size, const sockaddr* addr) {
  if (addr->sa_family == AF_INET) {
    auto addr4 = sockets::sockaddr_in_cast(addr);
    inet_ntop(AF_INET, &addr4->sin_addr, buf, addr_len);
  } else if (addr->sa_family == AF_INET6) {
    auto addr6 = sockets::sockaddr_in6_cast(addr);
    inet_ntop(AF_INET6, &addr6->sin6_addr, buf, addr_len);
  }
}

void sockets::from_Ip_port(const char* ip, uint16_t port, sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = host_to_network_16(port);
  if (inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    // TODO handle error;
  }
}
void sockets::from_Ip_port(const char* ip, uint16_t port, sockaddr_in6* addr) {
  addr->sin6_family = AF_INET6;
  addr->sin6_port = host_to_network_16(port);
  if (inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
    // TODO handle error;
  }
}

int sockets::get_socket_error(int sockfd) {
  int optval;
  socklen_t optlen = sizeof(optval);
  if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    // TODO handle error
  } else {
    return optval;
  }
}

sockaddr_in6 sockets::get_local_addr(int sockfd) {
  sockaddr_in6 local_addr;
  socklen_t addr_len = sizeof(local_addr);
  if (getsockname(sockfd, sockaddr_cast(&local_addr), &addr_len) < 0) {
    // TODO handle error
  }
  return local_addr;
}
sockaddr_in6 sockets::get_peer_addr(int sockfd) {
  sockaddr_in6 peer_addr;
  socklen_t addr_len = sizeof(peer_addr);
  if (getpeername(sockfd, sockaddr_cast(&peer_addr), &addr_len) < 0) {
    // TODO handle error
  }
  return peer_addr;
}

bool sockets::is_self_connect(int sockfd) {
  sockaddr_in6 local_addr = get_local_addr(sockfd);
  sockaddr_in6 peer_addr = get_peer_addr(sockfd);
  if (local_addr.sin6_family == AF_INET) {
    // TODO make the cast a template function
    sockaddr_in* local_addr_4 =
        static_cast<sockaddr_in*>(static_cast<void*>(&local_addr));
    sockaddr_in* peer_addr_4 =
        static_cast<sockaddr_in*>(static_cast<void*>(&peer_addr));
    return local_addr_4->sin_port == peer_addr_4->sin_port &&
           local_addr_4->sin_addr.s_addr == peer_addr_4->sin_addr.s_addr;

  } else if (local_addr.sin6_family == AF_INET6) {
    return local_addr.sin6_port == peer_addr.sin6_port &&
           (memcmp(&local_addr.sin6_addr, &peer_addr.sin6_addr,
                   sizeof(local_addr)) == 0);
  }
  return false;
}