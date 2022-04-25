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
  BlockingQueue() : _mutex(), _not_empty(_mutex), _queue() {}

  void push(const T& value) {
    MutexLockGuard lock(_mutex);
    _queue.push_back(value);
    _not_empty.notify();
  }
  void push(T&& value) {
    MutexLockGuard lock(_mutex);
    _queue.push_back(std::move(value));
    _not_empty.notify();
  }

  T pop() {
    MutexLockGuard lock(_mutex);
    while (_queue.empty()) {
      _not_empty.wait();
    }
    T front(std::move(_queue.front()));
    _queue.pop_front();
    return front;
  }

  // TODO change the function name
  queue_type drain() {
    queue_type new_queue;
    MutexLockGuard lock(_mutex);
    new_queue = std::move(_queue);
    return new_queue;
  }

  std::size_t size() const {
    MutexLockGuard lock(_mutex);
    return _queue.size();
  }
  bool empty() const {
    MutexLockGuard lock(_mutex);
    return _queue.empty();
  }

 private:
  mutable MutexLock _mutex;
  Condition _not_empty;
  queue_type _queue;
};

}  // namespace suduo
#endif