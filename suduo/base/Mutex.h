#ifndef MUTEX_H
#define MUTEX_H
#include <asm-generic/errno-base.h>
#include <pthread.h>
#include <sched.h>

#include <cassert>
#include <mutex>

#include "CurrentThreadInfo.h"
#include "iostream"
#include "suduo/base/Exception.h"
#include "suduo/base/noncopyable.h"

#ifndef NDEBUG
#define ERROR_CHECK(ret)                                            \
  ({                                                                \
    __typeof__(ret) errnum = (ret);                                 \
    if (__builtin_expect((errnum) != 0, 0))                         \
      __assert_perror_fail((errnum), __FILE__, __LINE__, __func__); \
  })
#else
#define ERROR_CHECK(ret) \
  ({                     \
    int errnum = (ret);  \
    assert(errnum == 0); \
  })
#endif

namespace suduo {
class MutexLock : noncopyable {
 public:
  MutexLock() : _holder(0) {
    ERROR_CHECK(pthread_mutex_init(&_mutex, nullptr));
  }
  ~MutexLock() {
    assert(_holder == 0);
    ERROR_CHECK(pthread_mutex_destroy(&_mutex));
  }

  void lock() {
    ERROR_CHECK(pthread_mutex_lock(&_mutex));
    set_holder();
  }

  void unlock() {
    ERROR_CHECK(pthread_mutex_unlock(&_mutex));
    unset_holder();
  }
  // FIXME change name ?
  //  used to check whether a mutex is locked
  bool try_lock() {
    if (pthread_mutex_trylock(&_mutex) != 0) {
      return false;
    }
    set_holder();
    return true;
  }
  // must be called when the mutex is locked
  bool is_locked_by_this_thread() const {
    return _holder == Current_thread_info::tid();
  }

  void assert_locked() const { assert(is_locked_by_this_thread()); }

  pthread_mutex_t* get_pthread_mutex() { return &_mutex; }

 private:
  friend class Condition;

  void set_holder() { _holder = Current_thread_info::tid(); }
  void unset_holder() { _holder = 0; }

  pthread_mutex_t _mutex;
  pid_t _holder;
};

class MutexLockGuard : noncopyable {
 public:
  explicit MutexLockGuard(MutexLock& mutex) : _mutex(mutex) { _mutex.lock(); }

  ~MutexLockGuard() { _mutex.unlock(); }

 private:
  MutexLock& _mutex;
};

}  // namespace suduo

#endif