#ifndef BOUND_BLOCKING_QUEUE_H
#define BOUND_BLOCKING_QUEUE_H
#include <cstdio>
#include <queue>

#include "CircularBuffer.h"
#include "suduo/base/Condition.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
template <typename T>
class BoundedBlockingQueue : noncopyable {
  using queue_type = suduo::detail::CircularBuffer<T>;

 public:
  explicit BoundedBlockingQueue(int max_size)
      : _mutex(), _queue(max_size), _not_empty(_mutex), _not_full(_mutex) {}

  void push(const T& val) {
    MutexLockGuard lock(_mutex);
    while (_queue.full()) {
      _not_full.wait();
    }
    _queue.push(val);
    _not_empty.notify();
  }
  void push(T&& val) {
    MutexLockGuard lock(_mutex);
    while (_queue.full()) {
      _not_full.wait();
    }
    _queue.push(std::move(val));
    _not_empty.notify();
  }

  T pop() {
    MutexLockGuard lock(_mutex);
    while (_queue.empty()) {
      _not_empty.wait();
    }
    T val = std::move(_queue.pop());
    _not_full.notify();
    return val;
  }

  bool empty() {
    MutexLockGuard lock(_mutex);
    return _queue.empty();
  }

  bool full() {
    MutexLockGuard lock(_mutex);
    return _queue.full();
  }

  int size() {
    MutexLockGuard lock(_mutex);
    return _queue.size();
  }

  int capacity() {
    MutexLockGuard lock(_mutex);
    return _queue.capacity();
  }

 private:
  mutable MutexLock _mutex;
  queue_type _queue;
  Condition _not_empty;
  Condition _not_full;
};
}  // namespace suduo
#endif