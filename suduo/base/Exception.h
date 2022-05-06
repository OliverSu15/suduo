#ifndef EXCEPTION_H
#define EXCEPTION_H
#include <exception>
#include <string>

#include "suduo/base/CurrentThreadInfo.h"
namespace suduo {
class Exception : std::exception {
 public:
  Exception(std::string what)
      : _message(std::move(what)),
        _stack_trace(Current_thread_info::stack_trace(false)) {}
  ~Exception() noexcept override = default;

  const char* what() const noexcept override { return _message.c_str(); }

  const char* stackTrace() const noexcept { return _stack_trace.c_str(); }

 private:
  std::string _message;
  std::string _stack_trace;
};

#define SUDUO_EXCEPTION_UNKNOW Exception("Unknow error")

}  // namespace suduo
#endif