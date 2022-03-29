#ifndef COUNTDOWNLATCH_H
#define COUNTDOWNLATCH_H
#include "suduo/base/Condition.h"
#include "suduo/base/Mutex.h"
namespace suduo {
class CountDownLatch {
 public:
  // TODO count can't smaller than 0
  explicit CountDownLatch(int count)
      : _mutex(), _condition(_mutex), _count(count) {}

  void wait() {
    MutexLockGuard lock(_mutex);
    while (_count > 0) _condition.wait();
  }

  void count_down() {
    MutexLockGuard lock(_mutex);
    _count--;
    if (_count == 0) _condition.notifyAll();
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