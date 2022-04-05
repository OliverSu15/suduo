#ifndef THREAD_POLL_H
#define THREAD_POLL_H
#include <cstddef>
#include <deque>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "suduo/base/Condition.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Thread.h"
namespace suduo {
class ThreadPoll {
 public:
  using Task = std::function<void()>;

  ThreadPoll(const std::string& name = "ThreadPoll");
  ~ThreadPoll();

  void start(int threads_num);
  void stop();

  void run(Task& task);
  // call before call start()
  void set_max_queue_size(int size) { max_queue_size = size; }
  void set_thread_init_func(Task& task) { thread_init_task = task; }

  size_t queue_size() const;

 private:
  void thread_func();
  ThreadPoll::Task get_next_task();
  bool is_full() const;

  mutable MutexLock _mutex;
  Condition not_full;
  Condition not_empty;

  std::string _name;

  Task thread_init_task;
  std::vector<std::unique_ptr<Thread>> thread_poll;

  std::deque<Task> task_queue;
  size_t max_queue_size;

  bool running;
};
}  // namespace suduo
#endif