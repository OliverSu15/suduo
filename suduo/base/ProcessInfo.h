#ifndef PROCESSINFO_H
#define PROCESSINFO_H
#include <sched.h>

#include <string>
namespace suduo {
namespace ProcessInfo {
pid_t pid();
std::string pid_string();
}  // namespace ProcessInfo
}  // namespace suduo
#endif