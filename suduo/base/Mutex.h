#ifndef MUTEX_H
#define MUTEX_H
#include <pthread.h>
#include <sched.h>

#include <cassert>
#include <mutex>

#include "CurrentThreadInfo.h"
namespace suduo {
// TODO can't copy can't move
class MutexLock {
 public:
  MutexLock() : holder(0) {
    int errnum = pthread_mutex_init(&mutex, nullptr);
    if (errnum) {
      // TODO error handle
    }
  }
  ~MutexLock() {
    // TODO 还是不太满意这个处理
    assert(holder == 0);
    int errnum = pthread_mutex_destroy(&mutex);
    if (errnum) {
      // TODO error handle
    }
  }

  void lock() {
    int errnum = pthread_mutex_lock(&mutex);
    if (errnum) {
      // TODO error handle
    }
    set_holder();
  }

  void unlock() {
    int errnum = pthread_mutex_unlock(&mutex);
    if (errnum) {
      // TODO error handle
    }
    unset_holder();
  }

  bool try_lock() {
    int errnum = pthread_mutex_trylock(&mutex);
    if (errnum) {
      // TODO error handle and return false if busy
    }
    set_holder();
  }

 private:
  inline void set_holder() { holder = Current_thread_info::tid(); }
  inline void unset_holder() { holder = 0; }

  pthread_mutex_t mutex;
  pid_t holder;
};

class MutexLockGurad {
 public:
  explicit MutexLockGurad(MutexLock& mutex) : _mutex(mutex) { _mutex.lock(); }

  ~MutexLockGurad() { _mutex.unlock(); }

 private:
  MutexLock& _mutex;
};

}  // namespace suduo

#endif