#include "suduo/net/InetAddress.h"

#include "suduo/base/Logger.h"

//#define BOOST_TEST_MODULE InetAddressTest
#define BOOST_TEST_MAIN
#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>

using suduo::string;
using suduo::net::InetAddress;

BOOST_AUTO_TEST_CASE(testInetAddress) {
  InetAddress addr0(1234);
  BOOST_CHECK_EQUAL(addr0.to_Ip(), string("0.0.0.0"));
  BOOST_CHECK_EQUAL(addr0.to_Ip_port(), string("0.0.0.0:1234"));
  BOOST_CHECK_EQUAL(addr0.port(), 1234);

  InetAddress addr1(4321, true);
  BOOST_CHECK_EQUAL(addr1.to_Ip(), string("127.0.0.1"));
  BOOST_CHECK_EQUAL(addr1.to_Ip_port(), string("127.0.0.1:4321"));
  BOOST_CHECK_EQUAL(addr1.port(), 4321);

  InetAddress addr2("1.2.3.4", 8888);
  BOOST_CHECK_EQUAL(addr2.to_Ip(), string("1.2.3.4"));
  BOOST_CHECK_EQUAL(addr2.to_Ip_port(), string("1.2.3.4:8888"));
  BOOST_CHECK_EQUAL(addr2.port(), 8888);

  InetAddress addr3("255.254.253.252", 65535);
  BOOST_CHECK_EQUAL(addr3.to_Ip(), string("255.254.253.252"));
  BOOST_CHECK_EQUAL(addr3.to_Ip_port(), string("255.254.253.252:65535"));
  BOOST_CHECK_EQUAL(addr3.port(), 65535);
}

BOOST_AUTO_TEST_CASE(testInet6Address) {
  InetAddress addr0(1234, false, true);
  BOOST_CHECK_EQUAL(addr0.to_Ip(), string("::"));
  BOOST_CHECK_EQUAL(addr0.to_Ip_port(), string("[::]:1234"));
  BOOST_CHECK_EQUAL(addr0.port(), 1234);

  InetAddress addr1(1234, true, true);
  BOOST_CHECK_EQUAL(addr1.to_Ip(), string("::1"));
  BOOST_CHECK_EQUAL(addr1.to_Ip_port(), string("[::1]:1234"));
  BOOST_CHECK_EQUAL(addr1.port(), 1234);

  InetAddress addr2("2001:db8::1", 8888, true);
  BOOST_CHECK_EQUAL(addr2.to_Ip(), string("2001:db8::1"));
  BOOST_CHECK_EQUAL(addr2.to_Ip_port(), string("[2001:db8::1]:8888"));
  BOOST_CHECK_EQUAL(addr2.port(), 8888);

  InetAddress addr3("fe80::1234:abcd:1", 8888, true);
  BOOST_CHECK_EQUAL(addr3.to_Ip(), string("fe80::1234:abcd:1"));
  BOOST_CHECK_EQUAL(addr3.to_Ip_port(), string("[fe80::1234:abcd:1]:8888"));
  BOOST_CHECK_EQUAL(addr3.port(), 8888);
}

BOOST_AUTO_TEST_CASE(testInetAddressResolve) {
  InetAddress addr(80);
  if (InetAddress::resolve("google.com", addr)) {
    LOG_INFO << "google.com resolved to " << addr.to_Ip_port();
  } else {
    LOG_ERROR << "Unable to resolve google.com";
  }
}
