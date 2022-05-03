#ifndef EVENTLOOP_THREAD_H
#define EVENTLOOP_THREAD_H
#include <functional>

#include "suduo/base/Condition.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Thread.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
namespace net {
class EventLoop;
class EventLoopThread : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                  const std::string& name = string());
  ~EventLoopThread();

  EventLoop* start_loop();

 private:
  void thread_func();

  EventLoop* _loop;
  bool _exiting;
  Thread _thread;
  MutexLock _mutex;
  Condition _cond;
  ThreadInitCallback _callback;
};
}  // namespace net
}  // namespace suduo

#endif