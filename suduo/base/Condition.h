#ifndef CONDITION_H
#define CONDITION_H
#include <bits/types/struct_timespec.h>
#include <bits/types/time_t.h>
#include <pthread.h>

#include <chrono>
#include <cstdint>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Mutex.h"
namespace suduo {
class Condition {
 public:
  explicit Condition(MutexLock& mutex_lock) : mutex(mutex_lock) {
    int errnum = pthread_cond_init(&cond, nullptr);
    if (errnum) {
      // TODO handle error
    }
  }

  ~Condition() {
    int errnum = pthread_cond_destroy(&cond);
    if (errnum) {
      // TODO handle error
    }
  }

  void notify() {
    int errnum = pthread_cond_signal(&cond);
    if (errnum) {
      // TODO handle error
    }
  }

  void notifyAll() {
    int errnum = pthread_cond_broadcast(&cond);
    if (errnum) {
      // TODO handle error
    }
  }

  void wait() {
    int errnum = pthread_cond_wait(&cond, mutex.get_pthread_mutex());
    if (errnum) {
      // TODO handle error
    }
  }

  void time_wait(std::chrono::nanoseconds wait_time) {
    timespec time = {0, 0};
    time.tv_sec = static_cast<time_t>(wait_time.count() / SECOND_TO_NANOSECOND);
    time.tv_nsec = static_cast<long>(wait_time.count() % SECOND_TO_NANOSECOND);
    int errnum =
        pthread_cond_timedwait(&cond, mutex.get_pthread_mutex(), &time);
    if (errnum) {
      // TODO handle error
    }
  }

 private:
  pthread_cond_t cond;
  MutexLock& mutex;
};
}  // namespace suduo

#endif