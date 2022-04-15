

#include "suduo/net/Socket.h"

#include <asm-generic/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/socket.h>

#include "suduo/base/Logger.h"
#include "suduo/net/InetAddress.h"
#include "suduo/net/SocketOpt.h"
using Socket = suduo::net::Socket;

Socket::~Socket() { sockets::close(_sock_fd); }

bool Socket::get_Tcp_info(tcp_info *info) const {
  // TODO make it a throw-exceptino type
  socklen_t len = sizeof(*info);
  return ::getsockopt(_sock_fd, SOL_TCP, TCP_INFO, info, &len) == 0;
}

bool Socket::get_Tcp_info_string(char *buf, int len) const {
  tcp_info info;
  bool ok = get_Tcp_info(&info);
  if (ok) {
    snprintf(
        buf, len,
        "unrecovered=%u "
        "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
        "lost=%u retrans=%u rtt=%u rttvar=%u "
        "sshthresh=%u cwnd=%u total_retrans=%u",
        info.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
        info.tcpi_rto,          // Retransmit timeout in usec
        info.tcpi_ato,          // Predicted tick of soft clock in usec
        info.tcpi_snd_mss, info.tcpi_rcv_mss,
        info.tcpi_lost,     // Lost packets
        info.tcpi_retrans,  // Retransmitted packets out
        info.tcpi_rtt,      // Smoothed round trip time in usec
        info.tcpi_rttvar,   // Medium deviation
        info.tcpi_snd_ssthresh, info.tcpi_snd_cwnd,
        info.tcpi_total_retrans);  // Total retransmits for entire connection
  }
  return ok;
}

void Socket::bind_address(const InetAddress &local_addr) {
  sockets::bind_or_abort(_sock_fd, local_addr.get_sock_addr());
}

void Socket::listen() { sockets::listen_or_abort(_sock_fd); }

int Socket::accept(InetAddress *peer_addr) {
  sockaddr_in6 addr;
  int connfd = sockets::accept(_sock_fd, &addr);
  if (connfd >= 0) {
    peer_addr->set_sock_addr_inet6(addr);
  }
  return connfd;
}

void Socket::shutdown_write() { sockets::shutdown_write(_sock_fd); }

void Socket::set_Tcp_no_delay(bool on) {
  int opt_val = on ? 1 : 0;  // TODO make it better
  setsockopt(_sock_fd, IPPROTO_TCP, TCP_NODELAY, &opt_val, sizeof(opt_val));
  // TODO check
}

void Socket::set_reuse_addr(bool on) {
  int opt_val = on ? 1 : 0;
  setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
  // TODO check
}

void Socket::set_reuse_port(bool on) {
  int opt_val = on ? 1 : 0;
  int ret =
      setsockopt(_sock_fd, SOL_SOCKET, SO_REUSEPORT, &opt_val, sizeof(opt_val));
  // TODO check
  if (ret < 0 && on) {
    // TODO handle error
    //  uduo::LOG_FATAL << "SO_REUSEPORT failed.";
  }
}

void Socket::set_keep_alive(bool on) {
  // TODO check
  int opt_val = on ? 1 : 0;
  ::setsockopt(_sock_fd, SOL_SOCKET, SO_KEEPALIVE, &opt_val, sizeof(opt_val));
}
