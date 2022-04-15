#ifndef TIMER_QUEUE_H
#define TIMER_QUEUE_H
#include <set>
#include <vector>

#include "suduo/base/Mutex.h"
#include "suduo/base/Timestamp.h"
#include "suduo/net/Callbacks.h"
#include "suduo/net/Channel.h"
namespace suduo {
namespace net {
class EventLoop;
class Timer;
class TimerID;
class TimerQueue : noncopyable {
 public:
  explicit TimerQueue(EventLoop* loop);
  ~TimerQueue();

  TimerID add_timer(TimerCallback cb, Timestamp when, double interval);

  void cancel(TimerID timer_id);

 private:
  using Entry = std::pair<Timestamp, Timer*>;
  using TimerList = std::set<Entry>;
  using ActiveTimer = std::pair<Timer*, int64_t>;
  using ActiveTimerSet = std::set<ActiveTimer>;

  void add_timer_in_loop(Timer* timer);
  void cancel_in_loop(TimerID timer_id);
  void handle_read();
  std::vector<Entry> get_expired(Timestamp now);
  void reset(const std::vector<Entry>& expired, Timestamp now);
  bool insert(Timer* timer);

  EventLoop* _loop;
  const int _timer_fd;
  Channel _timer_fd_channel;

  TimerList _timers;

  ActiveTimerSet _active_timers;
  bool _calling_expired_timers;
  ActiveTimerSet _canceling_timers;
};
}  // namespace net
}  // namespace suduo
#endif