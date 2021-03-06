#ifndef TIMER_QUEUE_H
#define TIMER_QUEUE_H

#include <cstdint>
#include <map>
#include <memory>
#include <set>
#include <vector>

#include "suduo/base/Timestamp.h"
#include "suduo/base/noncopyable.h"
#include "suduo/net/Callbacks.h"
#include "suduo/net/Channel.h"
#include "suduo/net/Timer.h"
namespace suduo {
namespace net {
class EventLoop;
class Timer;
class TimerQueue : noncopyable {
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  uint64_t add_timer(TimerCallback cb, const Timestamp& when, double interval);

  void cancel(uint64_t timer_id);

 private:
  using TimerID = std::pair<Timestamp, uint64_t>;
  using TimerMap = std::map<TimerID, std::unique_ptr<Timer>>;
  using ActiveTimerMap = std::map<uint64_t, const std::unique_ptr<Timer>&>;
  using CancelingTimerSet = std::set<uint64_t>;

  void handle_read();
  bool insert(std::unique_ptr<Timer> timer);
  void reset(std::vector<std::unique_ptr<Timer>>& expired, Timestamp now);
  void cancel_in_loop(uint64_t timer_id);
  void add_timer_in_loop(std::unique_ptr<Timer>& timer);

  EventLoop* _loop;
  const int _timer_fd;
  Channel _timer_fd_channel;

  TimerMap _timers;

  ActiveTimerMap _active_timers;
  bool _calling_expired_timers;
  CancelingTimerSet _canceling_timers;
};
}  // namespace net
}  // namespace suduo
#endif