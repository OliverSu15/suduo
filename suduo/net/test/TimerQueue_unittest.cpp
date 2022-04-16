#include <stdio.h>
#include <unistd.h>

#include "suduo/base/Thread.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/EventLoopThread.h"

using namespace suduo;
using namespace suduo::net;

int cnt = 0;
EventLoop* g_loop;

void printTid() {
  printf("pid = %d, tid = %d\n", getpid(), Current_thread_info::tid());
  printf("now %s\n", Timestamp::now().to_string().c_str());
}

void print(const char* msg) {
  printf("msg %s %s\n", Timestamp::now().to_string().c_str(), msg);
  if (++cnt == 20) {
    g_loop->quit();
  }
}

void cancel(TimerID timer) {
  g_loop->cancel(timer);
  printf("cancelled at %s\n", Timestamp::now().to_string().c_str());
}

int main() {
  printTid();
  sleep(1);
  {
    EventLoop loop;
    g_loop = &loop;

    print("main");
    loop.run_after(1, std::bind(print, "once1"));
    loop.run_after(1.5, std::bind(print, "once1.5"));
    loop.run_after(2.5, std::bind(print, "once2.5"));
    loop.run_after(3.5, std::bind(print, "once3.5"));
    TimerID t45 = loop.run_after(4.5, std::bind(print, "once4.5"));
    loop.run_after(4.2, std::bind(cancel, t45));
    loop.run_after(4.8, std::bind(cancel, t45));
    loop.run_every(2, std::bind(print, "every2"));
    TimerID t3 = loop.run_every(3, std::bind(print, "every3"));
    loop.run_after(9.001, std::bind(cancel, t3));

    loop.loop();
    print("main loop exits");
  }
  sleep(1);
  {
    EventLoopThread loopThread;
    EventLoop* loop = loopThread.start_loop();
    loop->run_after(2, printTid);
    sleep(3);
    print("thread loop exits");
  }
}
