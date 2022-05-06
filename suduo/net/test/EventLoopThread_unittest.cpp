#include "suduo/net/EventLoopThread.h"

#include <stdio.h>
#include <unistd.h>

#include "suduo/base/CountDownLatch.h"
#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Thread.h"
#include "suduo/net/EventLoop.h"

using namespace suduo;
using namespace suduo::net;

void print(EventLoop* p = NULL) {
  printf("print: pid = %d, tid = %d, loop = %p\n", getpid(),
         Current_thread_info::tid(), p);
}

void quit(EventLoop* p) {
  print(p);
  p->quit();
}

int main() {
  print();

  {
    EventLoopThread thr1;  // never start
  }

  {
    // dtor calls quit()
    EventLoopThread thr2;
    EventLoop* loop = thr2.start_loop();
    loop->run_in_loop(std::bind(print, loop));
    Current_thread_info::sleep_us(500 * 1000);
  }

  {
    // quit() before dtor
    EventLoopThread thr3;
    EventLoop* loop = thr3.start_loop();
    loop->run_in_loop(std::bind(quit, loop));
    Current_thread_info::sleep_us(500 * 1000);
  }
}
