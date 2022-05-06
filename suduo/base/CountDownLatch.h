#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H
#include "suduo/base/Condition.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
class CountDownLatch : noncopyable {
 public:
  explicit CountDownLatch(int count)
      : _mutex(), _condition(_mutex), _count(count) {}

  void wait() {
    MutexLockGuard lock(_mutex);
    while (_count > 0) _condition.wait();
  }

  void count_down() {
    MutexLockGuard lock(_mutex);
    _count--;
    if (_count <= 0) _condition.notifyAll();
  }

  int get_count() const {
    MutexLockGuard lock(_mutex);
    return _count;
  }

 private:
  mutable MutexLock _mutex;
  Condition _condition;
  int _count;
};
}  // namespace suduo

#endif