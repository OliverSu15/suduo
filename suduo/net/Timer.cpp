#include "Timer.h"

#include "suduo/base/Timestamp.h"
using Timer = suduo::net::Timer;

std::atomic_int64_t Timer::_created = 1;

void Timer::restart(Timestamp now) {
  if (_repeated) {
    _expertion_time = addTime(now, _interval);  // TODO change it
  } else {
    _expertion_time = Timestamp::invalid();
  }
}