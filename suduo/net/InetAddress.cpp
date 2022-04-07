#include "InetAddress.h"

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <array>
#include <cstdint>
#include <string>
#include <utility>
#include <variant>

#include "suduo/net/Endian.h"
#include "suduo/net/SocketOpt.h"

using InetAddress = suduo::net::InetAddress;
using namespace suduo::net;

static const in_addr_t inaddr_loopback = INADDR_LOOPBACK;
static const in_addr_t inaddr_any = INADDR_ANY;

InetAddress::InetAddress(uint16_t port, bool loop_back_only, bool ipv6) {
  if (ipv6) {
    sockaddr_in6 addr_6;
    addr_6.sin6_family = AF_INET6;
    addr_6.sin6_addr = loop_back_only ? in6addr_loopback : in6addr_any;
    addr_6.sin6_port = sockets::host_to_network_16(port);
    _addr = std::move(addr_6);
  } else {
    sockaddr_in addr_4;
    addr_4.sin_family = AF_INET;
    in_addr_t ip = loop_back_only ? inaddr_loopback : inaddr_any;
    addr_4.sin_addr.s_addr = sockets::host_to_network_32(ip);
    addr_4.sin_port = sockets::host_to_network_16(port);
    _addr = std::move(addr_4);
  }
}

InetAddress::InetAddress(const char* ip, uint16_t port, bool ipv6) {
  // TODO check if 'ipv6' is false, but ip is in ipv6 form
  if (ipv6) {
    sockaddr_in6 addr_6;
    sockets::from_Ip_port(ip, port, &addr_6);
    _addr = std::move(addr_6);
  } else {
    sockaddr_in addr_4;
    sockets::from_Ip_port(ip, port, &addr_4);
    _addr = std::move(addr_4);
  }
}

InetAddress::InetAddress(const std::string& ip, uint16_t port, bool ipv6)
    : InetAddress(ip.c_str(), port, ipv6) {}

sa_family_t InetAddress::family() const {
  if (_addr.index() == 0) {
    return std::get<sockaddr_in>(_addr).sin_family;
  }
  return std::get<sockaddr_in6>(_addr).sin6_family;
}
std::string InetAddress::to_Ip_port() const {
  char buf[64] = "";
  sockets::to_Ip_port(buf, sizeof(buf), get_sock_addr());
  return buf;
}

std::string InetAddress::to_Ip() const {
  char buf[64] = "";
  sockets::to_Ip(buf, sizeof(buf), get_sock_addr());
  return buf;
}

uint32_t InetAddress::ipv4_net_endian() const {
  return std::get<sockaddr_in>(_addr).sin_addr.s_addr;
}

uint16_t InetAddress::port() const {
  return sockets::network_to_host_16(port_net_enddian());
}
static __thread char t_resolveBuffer[64 * 1024];
bool InetAddress::resolve(const char* hostname, InetAddress* result) {
  // TODO figure out
  // TODO change it style
  hostent hent;
  hostent* he = nullptr;
  int herrno = 0;

  int ret = gethostbyname_r(hostname, &hent, t_resolveBuffer,
                            sizeof(t_resolveBuffer), &he, &herrno);
  if (ret == 0 && he != nullptr) {
    std::get<sockaddr_in>(result->_addr).sin_addr =
        *(static_cast<in_addr*>(static_cast<void*>(he->h_addr)));
    return true;
  } else {
    if (ret) {
      // TODO handle error
    }
    return false;
  }
}
void InetAddress::set_scope_id(uint32_t scope_id) {
  if (family() == AF_INET6) {
    std::get<sockaddr_in6>(_addr).sin6_scope_id = scope_id;
  }
}