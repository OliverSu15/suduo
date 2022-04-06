#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H
#include <cstddef>
#include <cstdio>
#include <queue>

#include "suduo/base/Condition.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/noncopyable.h"
namespace suduo {

template <typename T>
class BlockingQueue : noncopyable {
 public:
  using queue_type = std::deque<T>;
  BlockingQueue() : mutex(), not_empty(mutex) /*TODO not sure safe*/, queue() {}

  void push(const T& value) {
    MutexLockGuard lock(mutex);
    queue.push_back(value);
    not_empty.notify();
  }
  void push(T&& value) {
    MutexLockGuard lock(mutex);
    queue.push_back(std::move(value));
    not_empty.notify();
    // push_times++;
    //  std::printf("push:%d,pop:%d\n", push_times, pop_times);
  }

  T pop() {
    MutexLockGuard lock(mutex);
    while (queue.empty()) {
      not_empty.wait();
    }
    T front(std::move(queue.front()));
    queue.pop_front();
    // pop_times++;
    //  std::printf("push:%d,pop:%d\n", push_times, pop_times);
    return front;
  }

  // TODO change the function name
  queue_type drain() {
    queue_type new_queue;

    MutexLockGuard lock(mutex);
    new_queue = std::move(queue);

    return new_queue;
  }

  std::size_t size() const {
    MutexLockGuard lock(mutex);
    return queue.size();
  }
  bool empty() const {
    MutexLockGuard lock(mutex);
    return queue.empty();
  }

 private:
  mutable MutexLock mutex;
  Condition not_empty;
  queue_type queue;
  // int pop_times;
  // int push_times;
};

}  // namespace suduo
#endif