#ifndef INETADDRESS_H
#define INETADDRESS_H
#include <netinet/in.h>
#include <sys/socket.h>

#include <string>
#include <variant>

#include "suduo/base/copyable.h"
#include "suduo/net2/SocketOpt.h"
namespace suduo {
namespace net {
class InetAddress : public copyable {
 public:
  explicit InetAddress(uint16_t port, bool loop_back_only = false,
                       bool ipv6 = false);
  InetAddress(const std::string& ip, uint16_t port, bool ipv6 = false);
  explicit InetAddress(const sockaddr_in& addr) : _addr(addr) {}
  explicit InetAddress(const sockaddr_in6& addr) : _addr(addr) {}
  InetAddress() = default;

  sa_family_t family() const {
    if (_addr.index() == 0) {
      return std::get<sockaddr_in>(_addr).sin_family;
    }
    return std::get<sockaddr_in6>(_addr).sin6_family;
  }
  std::string to_Ip() const {
    std::array<char, 64> buf;
    sockets::to_Ip(buf.data(), buf.size(), get_sock_addr());
    return buf.data();
  }
  std::string to_Ip_port() const {
    std::array<char, 64> buf;
    sockets::to_Ip_port(buf.data(), buf.size(), get_sock_addr());
    return buf.data();
  }

  const sockaddr* get_sock_addr() const {
    if (_addr.index() == 0) {
      return sockets::sockaddr_cast(&(std::get<sockaddr_in>(_addr)));
    }
    return sockets::sockaddr_cast(&(std::get<sockaddr_in6>(_addr)));
  };

  void set_sock_addr(const sockaddr_in6& addr) { _addr = addr; }
  void set_sock_addr(const sockaddr_in& addr) { _addr = addr; }

  bool is_ipv6() const { return _addr.index() == 1; }

  static bool resolve(const std::string& hostname, InetAddress& result);

 private:
  using AddrType = std::variant<sockaddr_in, sockaddr_in6>;
  AddrType _addr;
};
}  // namespace net
}  // namespace suduo
#endif