#ifndef _LOG_FILE_H
#define _LOG_FILE_H
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
  LogFile(const std::string& base_name, int64_t roll_size,
          bool thread_safe = true, int flush_interval = 3,
          int check_every_N = 1024);
  ~LogFile();

  void append(const char* log, int len);
  void flush();
  bool roll_file();

 private:
  void append_unlocked(const char* log, int len);

  static std::string get_log_filename(const std::string& basename,
                                      Timestamp now);

  const std::string _basename;
  const off_t _roll_size;
  const int _flush_interval;
  const int _check_every_N;

  int _count;

  std::unique_ptr<MutexLock> _mutex;
  time_t _start_of_period;
  time_t _last_roll;
  time_t _last_flush;
  std::unique_ptr<FileUtil::FileAppender> _file;

  const static int kRollPerSeconds_ = 60 * 60 * 24;
};
}  // namespace suduo
#endif