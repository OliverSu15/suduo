#ifndef PROCESSINFO_H
#define PROCESSINFO_H
#include <sched.h>
#include <unistd.h>

#include <array>
#include <cassert>
#include <charconv>
#include <cstdint>
#include <string>
#include <system_error>

#include "suduo/base/Timestamp.h"
namespace suduo {
namespace ProcessInfo {
inline pid_t pid() { return getpid(); }
inline std::string pid_string() {
  std::array<char, 32> buf;
  auto [ptr, ec] =
      std::to_chars(buf.begin(), buf.end(), static_cast<int>(pid()));
  if (ec != std::errc()) std::printf("error\n");  // FIXME change the output
  return {buf.begin(), ptr};
}
}  // namespace ProcessInfo
}  // namespace suduo
#endif