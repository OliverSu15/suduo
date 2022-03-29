#ifndef CURRENT_THREAD_INFO_H
#define CURRENT_THREAD_INFO_H
#include <bits/types/struct_timespec.h>
#include <time.h>
#include <unistd.h>

#include <cstdint>
#include <cstdio>
#include <string>
namespace suduo {
const int MICROSECOND_TO_NANOSECOND = 1000;   // na -> us
const int MILLISECOND_TO_MICROSECOND = 1000;  // us -> ms
const int SECOND_TO_MILLISECOND = 1000;       // ms -> s
const int SECOND_TO_MICROSECOND =
    MILLISECOND_TO_MICROSECOND * SECOND_TO_MILLISECOND;  // us -> s
const int SECOND_TO_NANOSECOND =
    SECOND_TO_MICROSECOND * MICROSECOND_TO_NANOSECOND;
namespace Current_thread_info {
extern __thread int thread_cache_id;
extern __thread char thread_tid_string[32];
extern __thread int thread_tid_string_size;
extern __thread const char* thread_name;

inline void cache_thread_id() {
  thread_cache_id = gettid();
  thread_tid_string_size = std::snprintf(
      thread_tid_string, sizeof(thread_tid_string), "%5d", thread_cache_id);
}  // TODO 同步名字

inline int tid() {
  // TODO use __builtin_expect
  if (thread_cache_id == 0) cache_thread_id();
  return thread_cache_id;
}

inline const char* tid_string() { return thread_tid_string; }

inline int tid_string_length() { return thread_tid_string_size; }
inline const char* threads_name() { return thread_name; }

inline bool is_main_thread() { return tid() == getpid(); }

inline void sleep_us(int64_t us) {
  timespec time = {0, 0};
  time.tv_sec = static_cast<time_t>(us / (SECOND_TO_MICROSECOND));
  time.tv_nsec = static_cast<long>((us % SECOND_TO_MICROSECOND) *
                                   MICROSECOND_TO_NANOSECOND);
  nanosleep(&time, nullptr);
}

inline void sleep_ms(int64_t ms) { sleep_us(ms * MILLISECOND_TO_MICROSECOND); }

std::string stack_trace();
}  // namespace Current_thread_info
}  // namespace suduo

#endif