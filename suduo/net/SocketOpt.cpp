#include "SocketOpt.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>  // snprintf
#include <sys/socket.h>
#include <sys/uio.h>  // readv
#include <unistd.h>

#include "suduo/base/Logger.h"
#include "suduo/net/Endian.h"
// using namespace suduo::net::socket;
using namespace suduo::net;

int sockets::accept(int sockfd, sockaddr_in6* addr) {
  socklen_t len = addr_len;
  int connection_fd =
      accept4(sockfd, sockaddr_cast(addr), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connection_fd < 0) {
    LOG_SYSFATAL << "sockets::accept";
  }
  return connection_fd;
}

int sockets::accept(int sockfd, sockaddr_in* addr) {
  socklen_t len = addr_len;
  int connection_fd =
      accept4(sockfd, sockaddr_cast(addr), &len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  if (connection_fd < 0) {
    LOG_SYSFATAL << "sockets::accept";
  }
  return connection_fd;
}

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
    const auto addr4 = sockets::sockaddr_in_cast(addr);
    inet_ntop(AF_INET, &addr4->sin_addr, buf, addr_len);
  } else if (addr->sa_family == AF_INET6) {
    const auto addr6 = sockets::sockaddr_in6_cast(addr);
    inet_ntop(AF_INET6, &addr6->sin6_addr, buf, addr_len);
  }
}

void sockets::from_Ip_port(const char* ip, uint16_t port, sockaddr_in* addr) {
  addr->sin_family = AF_INET;
  addr->sin_port = host_to_network_16(port);
  if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0) {
    LOG_SYSERR << "sockets::from_Ip_port";
  }
}
void sockets::from_Ip_port(const char* ip, uint16_t port, sockaddr_in6* addr) {
  addr->sin6_family = AF_INET6;
  addr->sin6_port = host_to_network_16(port);
  if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0) {
    LOG_SYSERR << "sockets::from_Ip_port";
  }
}

int sockets::get_socket_error(int sockfd) {
  int optval;
  socklen_t optlen = sizeof(optval);
  if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
    LOG_SYSFATAL << "sockets::get_socket_error";
  } else {
    return optval;
  }
}

sockaddr_in6 sockets::get_local_addr(int sockfd) {
  sockaddr_in6 local_addr;
  socklen_t addr_len = sizeof(local_addr);
  if (getsockname(sockfd, sockaddr_cast(&local_addr), &addr_len) < 0) {
    LOG_SYSFATAL << "sockets::get_local_addr";
  }
  return local_addr;
}
sockaddr_in6 sockets::get_peer_addr(int sockfd) {
  sockaddr_in6 peer_addr;
  socklen_t addr_len = sizeof(peer_addr);
  if (getpeername(sockfd, sockaddr_cast(&peer_addr), &addr_len) < 0) {
    LOG_SYSFATAL << "sockets::get_peer_addr";
  }
  return peer_addr;
}

bool sockets::is_self_connect(int sockfd) {
  sockaddr_in6 local_addr = get_local_addr(sockfd);
  sockaddr_in6 peer_addr = get_peer_addr(sockfd);
  if (local_addr.sin6_family == AF_INET) {
    sockaddr_in* local_addr_4 =
        pointer_cast<sockaddr_in, sockaddr_in6>(&local_addr);
    sockaddr_in* peer_addr_4 =
        pointer_cast<sockaddr_in, sockaddr_in6>(&peer_addr);

    return local_addr_4->sin_port == peer_addr_4->sin_port &&
           local_addr_4->sin_addr.s_addr == peer_addr_4->sin_addr.s_addr;

  } else if (local_addr.sin6_family == AF_INET6) {
    return local_addr.sin6_port == peer_addr.sin6_port &&
           (memcmp(&local_addr.sin6_addr, &peer_addr.sin6_addr,
                   sizeof(local_addr)) == 0);
  }
  return false;
}