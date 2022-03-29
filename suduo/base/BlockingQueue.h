#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H
#include <cstddef>
#include <queue>

#include "suduo/base/Condition.h"
#include "suduo/base/Mutex.h"
namespace suduo {

template <typename T>
class BlockingQueue {
  using queue_type = std::queue<T>;

 public:
  BlockingQueue() : mutex(), not_empty(mutex) /*not sure safe*/, queue() {}

  void push(T& value) {
    MutexLockGuard lock(mutex);
    queue.push(value);
    not_empty.notify();
  }
  void push(T&& value) {
    MutexLockGuard lock(mutex);
    queue.push(std::move(value));
    not_empty.notify();
  }

  T pop() {
    MutexLockGuard lock(mutex);
    while (queue.empty()) {
      not_empty.wait();
    }
    T front = std::move(queue.front());
    queue.pop();
    return front;
  }

  // TODO change the function name
  queue_type drain() {
    queue_type new_queue;

    MutexLockGuard lock(mutex);
    new_queue = std::move(queue);

    return new_queue;
  }

  std::size_t size() const { return queue.size(); }
  bool empty() const { return queue.empty(); }

 private:
  mutable MutexLock mutex;
  Condition not_empty;
  queue_type queue;
};

}  // namespace suduo
#endif