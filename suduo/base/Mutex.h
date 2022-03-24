#ifndef MUTEX_H
#define MUTEX_H
#include <pthread.h>
#include <sched.h>

namespace suduo {
class MutexLock {
 public:
  MutexLock() : owner(0) { pthread_mutex_init(&mutex, nullptr); }
  ~MutexLock() { pthread_mutex_destroy(&mutex); }

  void lock() { pthread_mutex_lock(&mutex); }

  void unlock() { pthread_mutex_unlock(&mutex); }

  void trylock() { pthread_mutex_trylock(&mutex); }

 private:
  pthread_mutex_t mutex;
  pid_t owner;
};
}  // namespace suduo

#endif