#ifndef INETADDRESS_H
#define INETADDRESS_H
#include <netinet/in.h>
#include <sys/socket.h>

#include <cstddef>
#include <cstdint>
#include <string>
#include <variant>

#include "suduo/base/noncopyable.h"
namespace suduo {
namespace net {
class InetAddress {
 public:
  explicit InetAddress(uint16_t port, bool loop_back_only = false,
                       bool ipv6 = false);
  InetAddress(const char* ip, uint16_t port, bool ipv6 = false);
  InetAddress(const std::string& ip, uint16_t port, bool ipv6 = false);

  explicit InetAddress(const sockaddr_in& addr) : _addr(addr) {}
  explicit InetAddress(const sockaddr_in6& addr) : _addr(addr) {}

  sa_family_t family() const;
  std::string to_Ip() const;
  std::string to_Ip_port() const;
  uint16_t port() const;

  const sockaddr* get_sock_addr() const;
  void set_sock_addr_inet6(const sockaddr_in6& addr_6) { _addr = addr_6; }

  uint32_t ipv4_net_endian() const;
  uint32_t port_net_enddian() const;

  static bool resolve(const std::string& hostname, InetAddress* result);
  static bool resolve(const char* hostname, InetAddress* result);

  void set_scope_id(uint32_t scope_id);

 private:
  using AddrType = std::variant<sockaddr_in, sockaddr_in6>;
  AddrType _addr;
};

}  // namespace net
}  // namespace suduo
#endif