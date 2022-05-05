#include "suduo/net2/EventLoop.h"

#include <assert.h>
#include <stdio.h>
#include <unistd.h>

#include "suduo/base/Thread.h"

using namespace suduo;
using namespace suduo::net;

EventLoop* g_loop;

void callback() {
  printf("callback(): pid = %d, tid = %d\n", getpid(),
         Current_thread_info::tid());
  EventLoop anotherLoop;
}

void threadFunc() {
  printf("threadFunc(): pid = %d, tid = %d\n", getpid(),
         Current_thread_info::tid());

  assert(EventLoop::get_event_loop_of_current_thread() == NULL);
  EventLoop loop;
  assert(EventLoop::get_event_loop_of_current_thread() == &loop);
  loop.run_after(1.0, callback);
  loop.loop();
}

int main() {
  printf("main(): pid = %d, tid = %d\n", getpid(), Current_thread_info::tid());

  assert(EventLoop::get_event_loop_of_current_thread() == NULL);
  EventLoop loop;
  assert(EventLoop::get_event_loop_of_current_thread() == &loop);

  Thread thread(threadFunc);
  thread.start();

  loop.loop();
}
