#ifndef BOUND_BLOCKING_QUEUE_H
#define BOUND_BLOCKING_QUEUE_H
#include <cstdio>
#include <queue>

#include "CircularBuffer.h"
#include "suduo/base/Condition.h"
#include "suduo/base/Mutex.h"
namespace suduo {
template <typename T>
class BoundedBlockingQueue {
  using queue_type = suduo::detail::CircularBuffer<T>;

 public:
  explicit BoundedBlockingQueue(int max_size)
      : mutex(),
        queue(max_size),
        not_empty(mutex) /*TODO not sure safe*/,
        not_full(mutex) /*TODO not sure safe*/
  {}

  void push(const T& val) {
    MutexLockGuard lock(mutex);
    while (queue.full()) {
      not_full.wait();
    }
    queue.push(val);
    not_empty.notify();
  }
  void push(T&& val) {
    MutexLockGuard lock(mutex);
    while (queue.full()) {
      not_full.wait();
    }
    queue.push(std::move(val));
    not_empty.notify();
    // push_times++;
    // printf("%d\n", (push_times - pop_times) == queue.size());
  }

  T pop() {
    MutexLockGuard lock(mutex);
    while (queue.empty()) {
      not_empty.wait();
    }
    T val = std::move(queue.pop());
    not_full.notify();
    // pop_times++;
    // printf("%d\n", (push_times - pop_times) == queue.size());
    return val;
  }

  bool empty() {
    MutexLockGuard lock(mutex);
    return queue.empty();
  }

  bool full() {
    MutexLockGuard lock(mutex);
    return queue.full();
  }

  int size() {
    MutexLockGuard lock(mutex);
    return queue.size();
  }

  int capacity() {
    MutexLockGuard lock(mutex);
    return queue.capacity();
  }

 private:
  mutable MutexLock mutex;
  queue_type queue;
  Condition not_empty;
  Condition not_full;
  // int pop_times;
  // int push_times;
};
}  // namespace suduo
#endif