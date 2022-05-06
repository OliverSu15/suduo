#ifndef CONDITION_H
#define CONDITION_H
#include <asm-generic/errno.h>
#include <bits/types/struct_timespec.h>
#include <bits/types/time_t.h>
#include <pthread.h>

#include <chrono>
#include <cstdint>
#include <iostream>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Timestamp.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
class Condition : noncopyable {
 public:
  explicit Condition(MutexLock& mutex_lock) : _mutex(mutex_lock) {
    ERROR_CHECK(pthread_cond_init(&_cond, nullptr));
  }

  ~Condition() { ERROR_CHECK(pthread_cond_destroy(&_cond)); }

  void notify() { ERROR_CHECK(pthread_cond_signal(&_cond)); }

  void notifyAll() { ERROR_CHECK(pthread_cond_broadcast(&_cond)); }

  void wait() {
    UnassignGuard ug(_mutex);
    ERROR_CHECK(pthread_cond_wait(&_cond, _mutex.get_pthread_mutex()));
  }

  bool time_wait(const Timestamp& wait_time) {
    timespec time = Timestamp::to_time_spec(wait_time);
    UnassignGuard ug(_mutex);
    int errnum =
        pthread_cond_timedwait(&_cond, _mutex.get_pthread_mutex(), &time);
    return ETIMEDOUT == errnum;  // FIXME error handle
  }

 private:
  class UnassignGuard {
   public:
    explicit UnassignGuard(MutexLock& mutex) : _mutex(mutex) {
      _mutex.unset_holder();
    }
    ~UnassignGuard() { _mutex.set_holder(); }

   private:
    MutexLock& _mutex;
  };

 private:
  pthread_cond_t _cond;
  MutexLock& _mutex;
};
}  // namespace suduo

#endif