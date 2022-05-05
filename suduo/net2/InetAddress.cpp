#include "suduo/net2/InetAddress.h"

#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "suduo/net2/Endian.h"
using InetAddress = suduo::net::InetAddress;

static const in_addr_t inaddr_loopback = INADDR_LOOPBACK;
static const in_addr_t inaddr_any = INADDR_ANY;

InetAddress::InetAddress(uint16_t port, bool loop_back_only, bool ipv6) {
  if (ipv6) {
    sockaddr_in6 addr_6;
    addr_6.sin6_family = AF_INET6;
    addr_6.sin6_addr = loop_back_only ? in6addr_loopback : in6addr_any;
    addr_6.sin6_port = sockets::host_to_network_16(port);
    _addr = addr_6;
  } else {
    sockaddr_in addr_4;
    addr_4.sin_family = AF_INET;
    in_addr_t ip = loop_back_only ? inaddr_loopback : inaddr_any;
    addr_4.sin_addr.s_addr = sockets::host_to_network_32(ip);
    addr_4.sin_port = sockets::host_to_network_16(port);
    _addr = addr_4;
  }
}

InetAddress::InetAddress(const std::string& ip, uint16_t port, bool ipv6) {
  if (ipv6 || std::strchr(ip.c_str(), ':')) {
    sockaddr_in6 addr_6;
    sockets::from_Ip_port(ip.c_str(), port, &addr_6);
    _addr = addr_6;
  } else {
    sockaddr_in addr_4;
    sockets::from_Ip_port(ip.c_str(), port, &addr_4);
    _addr = addr_4;
  }
}

static __thread char t_resolveBuffer[64 * 1024];
bool InetAddress::resolve(const std::string& hostname, InetAddress& result) {
  hostent hent;
  hostent* he = nullptr;
  int herrno = 0;

  int ret = gethostbyname_r(hostname.c_str(), &hent, t_resolveBuffer,
                            sizeof(t_resolveBuffer), &he, &herrno);
  if (ret == 0 && he != nullptr) {
    std::get<sockaddr_in>(result._addr).sin_addr =
        *(static_cast<in_addr*>(static_cast<void*>(he->h_addr)));
    return true;
  } else {
    if (ret) {
      LOG_SYSERR << "InetAddress::resolve";
    }
    return false;
  }
}