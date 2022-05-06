#include <stdio.h>
#include <unistd.h>

#include "codec.h"
#include "suduo/base/Logger.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/TcpServer.h"

using std::placeholders::_1;
using std::placeholders::_2;
using std::placeholders::_3;
using suduo::get_pointer;

bool g_tcpNoDelay = false;

void onConnection(const suduo::net::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn->set_tcp_no_delay(g_tcpNoDelay);
  }
}

void onStringMessage(LengthHeaderCodec* codec,
                     const suduo::net::TcpConnectionPtr& conn,
                     const suduo::string& message, suduo::Timestamp) {
  codec->send(get_pointer(conn), message);
}

int main(int argc, char* argv[]) {
  if (argc > 1) {
    uint16_t port = static_cast<uint16_t>(atoi(argv[1]));
    g_tcpNoDelay = argc > 2 ? atoi(argv[2]) : false;
    int threadCount = argc > 3 ? atoi(argv[3]) : 0;

    LOG_INFO << "pid = " << getpid() << ", listen port = " << port;
    // suduo::Logger::setLogLevel(suduo::Logger::WARN);
    suduo::net::EventLoop loop;
    suduo::net::InetAddress listenAddr(port);
    suduo::net::TcpServer server(&loop, listenAddr, "PingPong");
    LengthHeaderCodec codec(std::bind(onStringMessage, &codec, _1, _2, _3));

    server.set_connection_callback(onConnection);
    server.set_message_callback(
        std::bind(&LengthHeaderCodec::onMessage, &codec, _1, _2, _3));

    if (threadCount > 1) {
      server.set_thread_num(threadCount);
    }

    server.start();

    loop.loop();
  } else {
    fprintf(stderr, "Usage: %s listen_port [tcp_no_delay [threads]]\n",
            argv[0]);
  }
}
