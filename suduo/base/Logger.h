#ifndef LOGGER_H
#define LOGGER_H
#include <cstddef>
#include <functional>

#include "suduo/base/LogStream.h"
#include "suduo/base/Thread.h"
#include "suduo/base/Timestamp.h"
namespace suduo {

class Logger {
 public:
  enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

  Logger(const char* source, int line);
  Logger(const char* source, int line, LogLevel level);
  Logger(const char* source, int line, LogLevel level, const char* func);
  Logger(const char* source, int line, bool to_abort);

  ~Logger();

  LogStream& stream() { return _stream; }

  using OutputFunc = std::function<void(const char*, size_t)>;
  using FlushFunc = std::function<void()>;
  using PreOutputFunc = std::function<void(LogStream&, const string&, int,
                                           LogLevel, const char*)>;
  using PostOutputFUnc =
      std::function<void(LogStream&, const string&, int, LogLevel)>;

  static void set_output_function(OutputFunc output_function);
  static void set_flush_function(FlushFunc flush_function);
  static void set_pre_output_function(PreOutputFunc pre_output_function);
  static void set_post_output_function(PostOutputFUnc post_output_function);
  static void set_log_level(Logger::LogLevel level);
  static LogLevel log_level();

 private:
  LogStream _stream;
  Timestamp _time;
  string _source;
  LogLevel _level;
  int _line;
};

}  // namespace suduo
#define LOG_TRACE                                         \
  if (suduo::Logger::log_level() <= suduo::Logger::TRACE) \
  suduo::Logger(__FILE__, __LINE__, suduo::Logger::TRACE, __func__).stream()
#define LOG_DEBUG                                         \
  if (suduo::Logger::log_level() <= suduo::Logger::DEBUG) \
  suduo::Logger(__FILE__, __LINE__, suduo::Logger::DEBUG, __func__).stream()
#define LOG_INFO                                         \
  if (suduo::Logger::log_level() <= suduo::Logger::INFO) \
  suduo::Logger(__FILE__, __LINE__).stream()
#define LOG_WARN suduo::Logger(__FILE__, __LINE__, suduo::Logger::WARN).stream()
#define LOG_ERROR \
  suduo::Logger(__FILE__, __LINE__, suduo::Logger::ERROR).stream()
#define LOG_FATAL \
  suduo::Logger(__FILE__, __LINE__, suduo::Logger::FATAL).stream()
#define LOG_SYSERR suduo::Logger(__FILE__, __LINE__, false).stream()
#define LOG_SYSFATAL suduo::Logger(__FILE__, __LINE__, true).stream()

#endif