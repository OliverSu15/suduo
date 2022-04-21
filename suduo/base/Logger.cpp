#include "Logger.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <ostream>
#include <string>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/LogStream.h"
#include "suduo/base/Timestamp.h"

// namespace suduo
using Logger = suduo::Logger;
namespace suduo {

// using namespace suduo::base;
const std::array<std::string, 6> LogLevel_name = {"TRACE", "DEBUG", "INFO",
                                                  "WARN",  "ERROR", "FATAL"};

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
void default_pre_output(LogStream& stream, const string& source, int line,
                        Logger::LogLevel level, const char* func) {
  stream << suduo::Timestamp::now().to_string() << " ";
  stream << Current_thread_info::tid_string() << " ";
  stream << LogLevel_name.at(level) << " ";
  if (func != nullptr) stream << func << ' ';
}
void default_post_output(LogStream& stream, const string& source, int line,
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

Logger::Logger(const char* source, int line)
    : _source(source),
      _line(line),
      _time(Timestamp::now()),
      _stream(),
      _level(INFO) {
  _source = _source.substr(_source.find_last_of('/') + 1);
  global_pre_output(_stream, _source, _line, _level, nullptr);
}

Logger::Logger(const char* source, int line, LogLevel level)
    : _source(source),
      _line(line),
      _level(level),
      _stream(),
      _time(Timestamp::now()) {
  _source = _source.substr(_source.find_last_of('/') + 1);
  global_pre_output(_stream, _source, _line, _level, nullptr);
}

Logger::Logger(const char* source, int line, bool to_abort)
    : _source(source),
      _line(line),
      _level(to_abort ? FATAL : ERROR),
      _stream(),
      _time(Timestamp::now()) {
  _source = _source.substr(_source.find_last_of('/') + 1);
  global_pre_output(_stream, _source, _line, _level, nullptr);
}
Logger::Logger(const char* source, int line, LogLevel level, const char* func)
    : _source(source),
      _line(line),
      _level(level),
      _stream(),
      _time(Timestamp::now()) {
  _source = _source.substr(_source.find_last_of('/') + 1);
  global_pre_output(_stream, _source, _line, _level, func);
}

Logger::~Logger() {
  global_post_output(_stream, _source, _line, _level);
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