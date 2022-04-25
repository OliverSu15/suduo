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
  explicit Condition(MutexLock& mutex_lock) : mutex(mutex_lock) {
    ERROR_CHECK(pthread_cond_init(&cond, nullptr));
  }

  ~Condition() { ERROR_CHECK(pthread_cond_destroy(&cond)); }

  void notify() { ERROR_CHECK(pthread_cond_signal(&cond)); }

  void notifyAll() { ERROR_CHECK(pthread_cond_broadcast(&cond)); }

  void wait() {
    UnassignGuard ug(mutex);
    ERROR_CHECK(pthread_cond_wait(&cond, mutex.get_pthread_mutex()));
  }

  bool time_wait(const Timestamp& wait_time) {
    timespec time = Timestamp::to_time_spec(wait_time);
    UnassignGuard ug(mutex);
    int errnum =
        pthread_cond_timedwait(&cond, mutex.get_pthread_mutex(), &time);
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
  pthread_cond_t cond;
  MutexLock& mutex;
};
}  // namespace suduo

#endif