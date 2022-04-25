#include "Logger.h"

#include <sys/time.h>

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iostream>
#include <ostream>
#include <string>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/LogStream.h"
#include "suduo/base/Timestamp.h"

// namespace suduo
using Logger = suduo::Logger;
namespace suduo {
__thread char thread_time[32];
__thread time_t thread_lastSecond;

inline LogStream& operator<<(LogStream& s, const Logger::SourceFile& v) {
  s.append(v.data_, v.size_);
  return s;
}

void time_to_string(const Timestamp& now) {
  time_t seconds = now.get_seconds_in_int64();
  if (seconds != thread_lastSecond) {
    thread_lastSecond = seconds;
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    int len =
        snprintf(thread_time, 64, "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 17);
  }
}
// using namespace suduo::base;
const char* LogLevel_name[6] = {"TRACE ", "DEBUG ", "INFO  ",
                                "WARN  ", "ERROR ", "FATAL "};

Logger::LogLevel initLogLevel() {
  if (::getenv("SUDUO_LOG_TRACE"))
    return Logger::TRACE;
  else if (::getenv("SUDUO_LOG_DEBUG"))
    return Logger::DEBUG;
  else
    return Logger::INFO;
}

void default_output(const char* val, size_t len) {
  size_t n = std::fwrite(val, 1, len, stdout);
  // FIXME error handle
}
void default_flush() { fflush(stdout); }
void default_pre_output(LogStream& stream, const Logger::SourceFile& source,
                        int line, const Timestamp& now, Logger::LogLevel level,
                        const char* func) {
  time_to_string(now);
  stream << thread_time << " ";
  stream << Current_thread_info::tid_string() << " ";
  stream << LogLevel_name[level];
  if (func != nullptr) stream << func << ' ';
}
void default_post_output(LogStream& stream, const Logger::SourceFile& source,
                         int line, const Timestamp& now,
                         Logger::LogLevel level) {
  stream << " -" << source << ":" << line << '\n';
}

Logger::OutputFunc global_output = default_output;
Logger::FlushFunc global_flush = default_flush;
Logger::LogLevel global_log_level = initLogLevel();
Logger::PreOutputFunc global_pre_output = default_pre_output;
Logger::PostOutputFUnc global_post_output = default_post_output;
}  // namespace suduo

Logger::LogLevel Logger::log_level() { return global_log_level; }

Logger::Logger(SourceFile source, int line)
    : _source(source),
      _line(line),
      _time(Timestamp::now()),
      _stream(),
      _level(INFO) {
  global_pre_output(_stream, _source, _line, _time, _level, nullptr);
}

Logger::Logger(SourceFile source, int line, LogLevel level)
    : _source(source),
      _line(line),
      _level(level),
      _stream(),
      _time(Timestamp::now()) {
  global_pre_output(_stream, _source, _line, _time, _level, nullptr);
}

Logger::Logger(SourceFile source, int line, bool to_abort)
    : _source(source),
      _line(line),
      _level(to_abort ? FATAL : ERROR),
      _stream(),
      _time(Timestamp::now()) {
  global_pre_output(_stream, _source, _line, _time, _level, nullptr);
}
Logger::Logger(SourceFile source, int line, LogLevel level, const char* func)
    : _source(source),
      _line(line),
      _level(level),
      _stream(),
      _time(Timestamp::now()) {
  global_pre_output(_stream, _source, _line, _time, _level, func);
}

Logger::~Logger() {
  global_post_output(_stream, _source, _line, _time, _level);
  global_output(_stream.buffer().data(), _stream.buffer().size());
  if (_level == FATAL) {
    global_flush();
    abort();
  }
}

void Logger::set_log_level(Logger::LogLevel level) { global_log_level = level; }
void Logger::set_output_function(OutputFunc output_function) {
  global_output = move(output_function);
}
void Logger::set_flush_function(FlushFunc flush_function) {
  global_flush = move(flush_function);
}
void Logger::set_pre_output_function(PreOutputFunc pre_output_function) {
  global_pre_output = move(pre_output_function);
}
void Logger::set_post_output_function(PostOutputFUnc post_output_function) {
  global_post_output = move(post_output_function);
}