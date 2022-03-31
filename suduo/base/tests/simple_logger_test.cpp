#include "suduo/base/Logger.h"

int main() {
  LOG_TRACE << "TRACE";
  LOG_DEBUG << "DEBUG";
  LOG_INFO << "INFO";
  LOG_WARN << "WARN";
  LOG_ERROR << "ERROR";
  LOG_FATAL << "FATAL";
  return 0;
}