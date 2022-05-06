#include "suduo/net/Socket.h"

#include <netinet/tcp.h>

#include "suduo/base/Logger.h"

using Socket = suduo::net::Socket;

bool Socket::get_TCP_info(tcp_info* info) const {
  socklen_t len = sizeof(*info);
  if (getsockopt(_sock_fd, SOL_TCP, TCP_INFO, &info, &len) < 0) {
    LOG_ERROR << "Socket::get_TCP_info()";
    return false;
  }
  return true;
}

bool Socket::get_TCP_info_string(char* buf, int len) const {
  tcp_info info;
  if (get_TCP_info(&info)) {
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
    return true;
  }
  return false;
}