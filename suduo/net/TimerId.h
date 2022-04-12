#ifndef TIMER_ID_H
#define TIMER_ID_H
#include <cstdint>

#include "suduo/net/TimerQueue.h"
namespace suduo {
namespace net {
class Timer;
class TimerID {
 public:
  TimerID() : _timer(nullptr), _sequence(0) {}
  TimerID(Timer* timer, int64_t sequence)
      : _timer(timer), _sequence(sequence) {}

 private:
  friend class TimerQueue;
  Timer* _timer;
  int64_t _sequence;
};
}  // namespace net
}  // namespace suduo
#endif