#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <atomic>
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
class ThreadPool : noncopyable {
 public:
  using Task = std::function<void()>;

  ThreadPool(const std::string& name = "ThreadPool");
  ~ThreadPool();

  void start(int threads_num);
  void stop();

  void run(Task task);
  // call before call start()
  void set_max_queue_size(int size) { max_queue_size = size; }
  void set_thread_init_func(Task& task) { thread_init_task = task; }

  size_t queue_size() const;

 private:
  void thread_func();
  ThreadPool::Task get_next_task();
  bool is_full() const;

  mutable MutexLock _mutex;
  Condition not_full;
  Condition not_empty;

  std::string _name;

  Task thread_init_task;
  std::vector<std::unique_ptr<Thread>> thread_pool;

  std::deque<Task> task_queue;
  size_t max_queue_size;

  std::atomic_bool running;
};
}  // namespace suduo
#endif