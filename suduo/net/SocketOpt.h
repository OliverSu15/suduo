#ifndef SOCKET_OPT_H
#define SOCKET_OPT_H
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <cstddef>
namespace suduo {
namespace net {
namespace sockets {
// make the biggest len as the len for every one
const socklen_t addr_len = sizeof(sockaddr_in6);

int create_Nonblocking_or_abort(sa_family_t family);

int connect(int sockfd, const sockaddr* addr);
void bind_or_abort(int sockfd, const sockaddr* addr);
void listen_or_abort(int sockfd);
int accept(int sockfd, sockaddr_in6* addr);  // TODO ?

ssize_t read(int sockfd, void* buf, size_t count);
ssize_t readv(int sockfd, const iovec* iov, int iovcnt);
ssize_t write(int sockfd, const void* buf, size_t count);

void close(int sockfd);
void shutdown_write(int sockfd);

void to_Ip_port(char* buf, size_t size, const sockaddr* addr);
void to_Ip(char* buf, size_t size, const sockaddr* addr);

void from_Ip_port(const char* ip, uint16_t port, sockaddr_in* addr);
void from_Ip_port(const char* ip, uint16_t port, sockaddr_in6* addr);

int get_socket_error(int sockfd);

const sockaddr* sockaddr_cast(const sockaddr_in* addr);
const sockaddr* sockaddr_cast(const sockaddr_in6* addr);
sockaddr* sockaddr_cast(sockaddr_in6* addr);

const sockaddr_in* sockaddr_in_cast(const sockaddr* addr);
const sockaddr_in6* sockaddr_in6_cast(const sockaddr* addr);

sockaddr_in6 get_local_addr(int sockfd);
sockaddr_in6 get_peer_addr(int sockfd);
bool is_self_connect(int sockfd);

}  // namespace sockets
}  // namespace net
}  // namespace suduo
#endif