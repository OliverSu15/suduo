#include "CurrentThreadInfo.h"

#include <sched.h>

#include <type_traits>
namespace suduo {
namespace Current_thread_info {
__thread int thread_cache_id = 0;
__thread char thread_tid_string[32];
__thread int thread_tid_string_size = 6;
__thread const char* thread_name = "unknow";

static_assert(std::is_same<pid_t, int>::value, "pid_t should be int");
}  // namespace Current_thread_info
}  // namespace suduo