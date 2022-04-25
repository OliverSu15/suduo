#include "LogFile.h"

#include <chrono>
#include <cstdio>

#include "suduo/base/FileUtil.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/ProcessInfo.h"
#include "suduo/base/Timestamp.h"
using LogFile = suduo::LogFile;

LogFile::LogFile(const std::string& base_name, int64_t roll_size,
                 bool thread_safe, int flush_interval, int check_every_N)
    : _basename(base_name),
      _roll_size(roll_size),
      _flush_interval(Timestamp::Seconds(flush_interval)),
      _check_every_N_times(check_every_N),
      _count(0),
      _mutex(thread_safe ? new MutexLock : nullptr),
      _start_of_period(Timestamp::Seconds(0)),
      _last_roll(Timestamp::Seconds(0)),
      _last_flush(Timestamp::Seconds(0)) {
  roll_file();
}

LogFile::~LogFile() = default;

void LogFile::append(const char* log, size_t len) {
  if (_mutex) {
    MutexLockGuard lock(*_mutex);
    append_unlocked(log, len);
  } else {
    append_unlocked(log, len);
  }
}

void LogFile::flush() {
  if (_mutex) {
    MutexLockGuard lock(*_mutex);
    _file->flush();
  } else {
    _file->flush();
  }
}

void LogFile::append_unlocked(const char* log, size_t len) {
  _file->append(log, len);
  if (_file->written_bytes() > _roll_size) {
    roll_file();
  } else {
    ++_count;
    if (_count >= _check_every_N_times) {
      _count = 0;
      Timestamp now = Timestamp::now();
      Timestamp thisPeriod_{Timestamp::Seconds(
          now.get_seconds_in_int64() / roll_per_seconds_ * roll_per_seconds_)};
      if (thisPeriod_ != _start_of_period) {
        roll_file();
      } else if (now - _last_flush > _flush_interval) {
        _last_flush = now;
        _file->flush();
      }
    }
  }
}

bool LogFile::roll_file() {
  Timestamp now = Timestamp::now();
  Timestamp start{Timestamp::Seconds(now.get_seconds_in_int64() /
                                     roll_per_seconds_ * roll_per_seconds_)};

  if (now > _last_roll) {
    std::string filename = get_log_filename(_basename, now);
    _last_roll = now;
    _last_flush = now;
    _start_of_period = start;
    _file.reset(new FileUtil::FileAppender(filename));
    return true;
  }
  return false;
}

std::string LogFile::get_log_filename(const std::string& basename,
                                      const Timestamp& now) {
  std::string filename;
  filename.reserve(basename.size() + 32);
  filename += basename + ".";
  filename += now.to_log_string() + ".";
  filename += suduo::ProcessInfo::pid_string();
  filename += ".log";
  return filename;
}