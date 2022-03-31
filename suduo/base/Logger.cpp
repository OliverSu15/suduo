#include "Logger.h"

#include <array>
#include <cstddef>
#include <cstdio>
#include <iostream>
#include <ostream>
#include <string>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/LogStream.h"
#include "suduo/base/Timestamp.h"

std::array<std::string, 6> LogLevelName = {"TRACE", "DEBUG", "INFO",
                                           "WARN",  "ERROR", "FATAL"};

using Logger = suduo::Logger;

void default_output(const char* val, int len) {
  // TODO change later
  size_t n = std::fwrite(val, sizeof val[0], len, stdout);
}

Logger::Logger(const char* source, int line)
    : _source(source),
      _line(line),
      _time(Timestamp::now()),
      _stream(),
      _level(INFO) {
  _source = _source.substr(_source.find_last_of('/') + 1);
  Current_thread_info::tid();
  _stream << _time.to_string();
  _stream << Current_thread_info::tid_string() << " ";
  _stream << LogLevelName[_level] << " ";
}

Logger::Logger(const char* source, int line, LogLevel level)
    : _source(source),
      _line(line),
      _level(level),
      _stream(),
      _time(Timestamp::now()) {
  _source = _source.substr(_source.find_last_of('/') + 1);
  Current_thread_info::tid();
  _stream << _time.to_string();
  _stream << Current_thread_info::tid_string() << " ";
  _stream << LogLevelName[_level] << " ";
}

Logger::~Logger() {
  _stream << " -" << _source << ":" << _line << '\n';
  //   for (int i = 0; i < _stream.buffer().size(); i++) {
  //     std::cout << *(_stream.buffer().data() + i) << std::endl;
  //   }
  default_output(_stream.buffer().data(), _stream.buffer().size());
}

void Logger::operator<<(suduo::LogStream& stream) {
  _stream.append(stream.buffer().data(), stream.buffer().size());
}
