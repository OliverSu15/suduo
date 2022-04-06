#ifndef INETADDRESS_H
#define INETADDRESS_H
#include <netinet/in.h>

#include <cstdint>
#include <string>
#include <variant>

#include "suduo/base/noncopyable.h"
namespace suduo {
namespace net {
class InetAddress : noncopyable {
 public:
  explicit InetAddress(uint16_t port, bool loop_back_onlt = false,
                       bool ipv6 = false);
  InetAddress(const char* ip, uint16_t port, bool ipv6 = false);
  InetAddress(const std::string& ip, uint16_t port, bool ipv6 = false);

  explicit InetAddress(const sockaddr_in& addr) : _addr(addr) {}
  explicit InetAddress(const sockaddr_in6& addr) : _addr(addr) {}

 private:
  using AddrType = std::variant<sockaddr_in, sockaddr_in6>;

  AddrType _addr;
};

}  // namespace net
}  // namespace suduo
#endif