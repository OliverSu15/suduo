#include <stdio.h>
#include <unistd.h>

#include <atomic>
#include <utility>

#include "suduo/base/Logger.h"
#include "suduo/base/Thread.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/InetAddress.h"
#include "suduo/net/TcpServer.h"

using namespace suduo;
using namespace suduo::net;

void onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn->set_tcp_no_delay(true);
  }
}

void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
  conn->send(buf);
}

int main(int argc, char* argv[]) {
  if (argc < 4) {
    fprintf(stderr, "Usage: server <address> <port> <threads>\n");
  } else {
    LOG_INFO << "pid = " << getpid()
             << ", tid = " << Current_thread_info::tid();
    Logger::set_log_level(Logger::WARN);

    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    InetAddress listenAddr(ip, port);
    int threadCount = atoi(argv[3]);

    EventLoop loop;

    TcpServer server(&loop, listenAddr, "PingPong");

    server.set_connection_callback(onConnection);
    server.set_message_callback(onMessage);

    if (threadCount > 1) {
      server.set_thread_num(threadCount);
    }

    server.start();

    loop.loop();
  }
}
