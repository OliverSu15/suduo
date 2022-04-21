#ifndef PROCESSINFO_H
#define PROCESSINFO_H
#include <sched.h>
#include <unistd.h>

#include <charconv>
#include <cstdint>
#include <string>
#include <system_error>

#include "suduo/base/Timestamp.h"
namespace suduo {
namespace ProcessInfo {
inline pid_t pid() { return getpid(); }
inline std::string pid_string() {
  string buf;
  buf.reserve(32);
  auto [ptr, ec] = std::to_chars(buf.begin().base(), buf.end().base(),
                                 static_cast<int>(pid()));
  if (ec != std::errc()) std::printf("error\n");  // FIXME change the output
  return buf;
}
}  // namespace ProcessInfo
}  // namespace suduo
#endif