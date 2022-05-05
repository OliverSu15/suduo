// TcpClient destructs when TcpConnection is connected but unique.

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Logger.h"
#include "suduo/base/Thread.h"
#include "suduo/net2/EventLoop.h"
#include "suduo/net2/TcpClient.h"

using namespace suduo;
using namespace suduo::net;

void threadFunc(EventLoop* loop) {
  InetAddress serverAddr("127.0.0.1", 1234);  // should succeed
  TcpClient client(loop, serverAddr, "TcpClient");
  client.connect();

  Current_thread_info::sleep_us(1000 * 1000);
  // client destructs when connected.
}

int main(int argc, char* argv[]) {
  Logger::set_log_level(Logger::DEBUG);

  EventLoop loop;
  loop.run_after(3.0, std::bind(&EventLoop::quit, &loop));
  Thread thr(std::bind(threadFunc, &loop));
  thr.start();
  loop.loop();
}
