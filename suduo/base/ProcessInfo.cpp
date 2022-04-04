#include "ProcessInfo.h"

#include <sched.h>
#include <unistd.h>
using namespace suduo;

pid_t ProcessInfo::pid() { return getpid(); }

// TODO change later
std::string ProcessInfo::pid_string() {
  char buf[32];
  snprintf(buf, sizeof buf, "%d", pid());
  return buf;
}
