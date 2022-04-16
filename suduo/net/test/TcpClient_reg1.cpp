// TcpClient::stop() called in the same iteration of IO event

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Logger.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/TcpClient.h"

using namespace suduo;
using namespace suduo::net;

TcpClient* g_client;

void timeout() {
  LOG_INFO << "timeout";
  g_client->stop();
}

int main(int argc, char* argv[]) {
  EventLoop loop;
  InetAddress serverAddr("127.0.0.1", 2);  // no such server
  TcpClient client(&loop, serverAddr, "TcpClient");
  g_client = &client;
  loop.run_after(0.0, timeout);
  loop.run_after(1.0, std::bind(&EventLoop::quit, &loop));
  client.connect();
  Current_thread_info::sleep_us(100 * 1000);
  loop.loop();
}
