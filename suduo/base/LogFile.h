#ifndef _LOG_FILE_H
#define _LOG_FILE_H
#include <cstddef>
#include <cstdint>
#include <memory>

#include "suduo/base/FileUtil.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Timestamp.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
// TODO change later
class LogFile : noncopyable {
 public:
  LogFile(const string& base_name, int64_t roll_size, bool thread_safe = true,
          int flush_interval = 3, int check_every_N = 1024);
  ~LogFile();

  void append(const char* log, size_t len);
  void flush();
  bool roll_file();

 private:
  void append_unlocked(const char* log, size_t len);

  static string get_log_filename(const string& basename, const Timestamp& now);

  const string _basename;
  const off_t _roll_size;
  const Timestamp _flush_interval;
  const int _check_every_N_times;

  int _count;

  std::unique_ptr<MutexLock> _mutex;
  Timestamp _start_of_period;
  Timestamp _last_roll;
  Timestamp _last_flush;
  std::unique_ptr<FileUtil::FileAppender> _file;

  const static int roll_per_seconds_ = 60 * 60 * 24;
};
}  // namespace suduo
#endif