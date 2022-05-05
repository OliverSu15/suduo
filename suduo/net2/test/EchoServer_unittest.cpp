#include <stdio.h>
#include <unistd.h>

#include <utility>

#include "suduo/base/Logger.h"
#include "suduo/base/Thread.h"
#include "suduo/net2/EventLoop.h"
#include "suduo/net2/InetAddress.h"
#include "suduo/net2/TcpServer.h"

using namespace suduo;
using namespace suduo::net;

int numThreads = 0;

class EchoServer {
 public:
  EchoServer(EventLoop* loop, const InetAddress& listenAddr)
      : loop_(loop), server_(loop, listenAddr, "EchoServer") {
    server_.set_connection_callback(
        std::bind(&EchoServer::onConnection, this, std::placeholders::_1));
    server_.set_message_callback(
        std::bind(&EchoServer::onMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
    server_.set_thread_num(numThreads);
  }

  void start() { server_.start(); }
  // void stop();

 private:
  void onConnection(const TcpConnectionPtr& conn) {
    LOG_TRACE << conn->peer_address().to_Ip_port() << " -> "
              << conn->local_address().to_Ip_port() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    LOG_INFO << conn->get_tcp_info_string();

    conn->send("hello\n");
  }

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp time) {
    string msg(buf->retrieve_all_as_string());
    LOG_TRACE << conn->name() << " recv " << msg.size() << " bytes at "
              << time.to_string();
    if (msg == "exit\n") {
      conn->send("bye\n");
      conn->shutdown();
    }
    if (msg == "quit\n") {
      loop_->quit();
    }
    conn->send(msg);
  }

  EventLoop* loop_;
  TcpServer server_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << Current_thread_info::tid();
  LOG_INFO << "sizeof TcpConnection = " << sizeof(TcpConnection);
  if (argc > 1) {
    numThreads = atoi(argv[1]);
  }
  bool ipv6 = argc > 2;
  EventLoop loop;
  InetAddress listenAddr(2000, false, ipv6);
  EchoServer server(&loop, listenAddr);

  server.start();

  loop.loop();
}
