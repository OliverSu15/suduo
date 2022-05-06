#ifndef CURRENT_THREAD_INFO_H
#define CURRENT_THREAD_INFO_H
#include <bits/types/struct_timespec.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <time.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <string>

#include "suduo/base/Timestamp.h"
namespace suduo {

namespace Current_thread_info {
extern __thread int thread_cache_id;
extern __thread char thread_tid_string[32];
extern __thread int thread_tid_string_size;
extern __thread const char* thread_name;
inline void cache_thread_id() {
  thread_cache_id = gettid();
  thread_tid_string_size = std::snprintf(
      thread_tid_string, sizeof(thread_tid_string), "%5d", thread_cache_id);
}
inline int tid() {
  // TODO use __builtin_expect
  // TODO current cache ability don't work, try to find a new one
  if (thread_cache_id == 0) cache_thread_id();
  return thread_cache_id;
}
// TODO make it can be called alone
inline const char* tid_string() {
  if (thread_cache_id == 0) cache_thread_id();
  return thread_tid_string;
}

inline int tid_string_length() { return thread_tid_string_size; }
inline const char* threads_name() { return thread_name; }

inline bool is_main_thread() { return tid() == getpid(); }

inline void sleep_us(int64_t us) {
  timespec time = Timestamp::to_time_spec({Timestamp::Microseconds(us)});
  nanosleep(&time, nullptr);
}

inline void sleep_ms(int64_t ms) { sleep_us(ms * MILLISECOND_TO_MICROSECOND); }

inline const char* name() { return thread_name; }

inline void after_fork() {
  Current_thread_info::thread_cache_id = 0;
  Current_thread_info::thread_name = "thread";
  Current_thread_info::tid();
}

class ThreadIDNameInitializer {
 public:
  ThreadIDNameInitializer() {
    Current_thread_info::thread_name = "thread";
    Current_thread_info::tid();
    pthread_atfork(nullptr, nullptr, &after_fork);
  }
};

inline ThreadIDNameInitializer initer;

std::string stack_trace(bool demangle);
}  // namespace Current_thread_info
}  // namespace suduo

#endif