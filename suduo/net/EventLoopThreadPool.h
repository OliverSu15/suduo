#ifndef EVENTLOOP_THREAD_POOL_H
#define EVENTLOOP_THREAD_POOL_H
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "suduo/base/noncopyable.h"
namespace suduo {
namespace net {
class EventLoop;
class EventLoopThread;
class EventLoopThreadPool : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;

  EventLoopThreadPool(EventLoop* base_loop,
                      const std::string& name = "EventLoopThreadPool");
  ~EventLoopThreadPool();

  void set_thread_num(int thread_num) { _thread_nums = thread_num; }

  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  EventLoop* get_next_loop();

  std::vector<EventLoop*> get_all_loops();

  bool started() const { return _started; }

  const std::string& name() const { return _name; }

 private:
  EventLoop* _base_loop;
  std::string _name;
  bool _started;
  int _thread_nums;
  int _next;
  std::vector<std::unique_ptr<EventLoopThread>> _threads;
  std::vector<EventLoop*> _loops;
};
}  // namespace net
}  // namespace suduo
#endif