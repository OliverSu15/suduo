#ifndef LOGGER_H
#define LOGGER_H
#include <functional>

#include "suduo/base/LogStream.h"
#include "suduo/base/Thread.h"
#include "suduo/base/Timestamp.h"
namespace suduo {
class Logger {
 public:
  enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

  explicit Logger(const char* source, int line);
  explicit Logger(const char* source, int line, LogLevel level);

  ~Logger();

  void operator<<(LogStream& stream);

  LogStream& stream() { return _stream; }

  using OutputFunc = std::function<void(const char*, int)>;
  using FlushFunc = std::function<void()>;

  static void set_output_function(OutputFunc output_function);
  static void set_flush_function(FlushFunc flush_function);

 private:
  LogStream _stream;
  Timestamp _time;
  string _source;
  LogLevel _level;
  int _line;
};
}  // namespace suduo
#define LOG_TRACE \
  suduo::Logger(__FILE__, __LINE__, suduo::Logger::TRACE).stream()
#define LOG_DEBUG \
  suduo::Logger(__FILE__, __LINE__, suduo::Logger::DEBUG).stream()
#define LOG_INFO suduo::Logger(__FILE__, __LINE__, suduo::Logger::INFO).stream()
#define LOG_WARN suduo::Logger(__FILE__, __LINE__, suduo::Logger::WARN).stream()
#define LOG_ERROR \
  suduo::Logger(__FILE__, __LINE__, suduo::Logger::ERROR).stream()
#define LOG_FATAL \
  suduo::Logger(__FILE__, __LINE__, suduo::Logger::FATAL).stream()

#endif