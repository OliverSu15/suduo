#include "InetAddress.h"

using InetAddress = suduo::net::InetAddress;

InetAddress::InetAddress(const char* ip, uint16_t port, bool ipv6) {
  // TODO check if 'ipv6' is false, but ip is in ipv6 form
  if (ipv6) {
  } else {
  }
}

InetAddress::InetAddress(const std::string& ip, uint16_t port, bool ipv6)
    : InetAddress(ip.c_str(), port, ipv6) {}