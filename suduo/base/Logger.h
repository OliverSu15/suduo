#ifndef LOGGER_H
#define LOGGER_H
#include "suduo/base/LogStream.h"
namespace suduo {
class Logger {
 public:
  enum LogLevel { TRACE, DEBUG, INFO, WARN, ERROR, FATAL };

 private:
  LogStream _stream;
};
}  // namespace suduo

#endif