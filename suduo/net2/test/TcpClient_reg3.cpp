// TcpClient destructs in a different thread.

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Logger.h"
#include "suduo/net2/EventLoopThread.h"
#include "suduo/net2/TcpClient.h"

using namespace suduo;
using namespace suduo::net;

int main(int argc, char* argv[]) {
  Logger::set_log_level(Logger::DEBUG);

  EventLoopThread loopThread;
  {
    InetAddress serverAddr("127.0.0.1", 1234);  // should succeed
    TcpClient client(loopThread.start_loop(), serverAddr, "TcpClient");
    client.connect();
    Current_thread_info::sleep_us(500 * 1000);  // wait for connect
    client.disconnect();
  }

  Current_thread_info::sleep_us(1000 * 1000);
}
