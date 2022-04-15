#ifndef EVENT_LOOP_THREAD_POOL_H
#define EVENT_LOOP_THREAD_POOL_H
#include <cstddef>
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

  EventLoopThreadPool(EventLoop* base_loop, const std::string& name);
  ~EventLoopThreadPool();

  void set_thread_num(int thread_num) { _thread_nums = thread_num; }
  void start(const ThreadInitCallback& cb = ThreadInitCallback());

  EventLoop* get_next_loop();

  EventLoop* get_loop_for_hash(size_t hash_code);

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