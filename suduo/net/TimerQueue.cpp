#include "TimerQueue.h"

#include <sys/timerfd.h>
#include <unistd.h>

#include "suduo/base/Logger.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/Timer.h"
#include "suduo/net/TimerId.h"

namespace suduo {
inline void memZero(void* p, size_t n) { memset(p, 0, n); }
namespace net {
namespace detail {
int create_timer_fd() {
  int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer_fd < 0) {
    // TODO handle error
  }
  return timer_fd;
}

timespec how_much_time_from_now(Timestamp when) {
  int64_t microseconds = when.get_microseconds_in_int64() -
                         Timestamp::now().get_microseconds_in_int64();
  if (microseconds < 100) {
    microseconds = 100;
  }
  timespec ts;
  ts.tv_sec = static_cast<time_t>(microseconds / SECOND_TO_MICROSECOND);
  ts.tv_nsec = (microseconds % SECOND_TO_MICROSECOND) * 1000;
  return ts;
}

void read_timer_fd(int timer_fd, Timestamp now) {
  uint64_t how_many;
  ssize_t n = read(timer_fd, &how_many, sizeof(how_many));
  LOG_TRACE << "TimerQueue::handleRead() " << how_many << " at "
            << now.to_string();
  if (n != sizeof(how_many)) {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n
              << " bytes instead of 8";
  }
}

void reset_timer_fd(int timer_fd, Timestamp expiration) {
  itimerspec new_value;
  itimerspec old_value;
  memZero(&new_value, sizeof new_value);
  memZero(&old_value, sizeof old_value);
  new_value.it_value = how_much_time_from_now(expiration);
  int ret = ::timerfd_settime(timer_fd, 0, &new_value, &old_value);
  if (ret) {
    // TODO handle error
    LOG_SYSERR << "timerfd_settime()";
  }
}

}  // namespace detail
}  // namespace net
}  // namespace suduo
using namespace suduo;
using TimerQueue = suduo::net::TimerQueue;
using namespace suduo::net::detail;

TimerQueue::TimerQueue(EventLoop* loop)
    : _loop(loop),
      _timer_fd(create_timer_fd()),
      _timer_fd_channel(loop, _timer_fd),
      _timers(),
      _calling_expired_timers(false) {
  _timer_fd_channel.set_read_callback(
      std::bind(&TimerQueue::handle_read, this));
  _timer_fd_channel.enable_reading();
}

TimerQueue::~TimerQueue() {
  _timer_fd_channel.disable_all();
  _timer_fd_channel.remove();
  close(_timer_fd);
  for (const Entry& timer : _timers) delete timer.second;  // TODO
}

suduo::net::TimerID TimerQueue::add_timer(TimerCallback cb, Timestamp when,
                                          double interval) {
  Timer* timer = new Timer(std::move(cb), when, interval);
  _loop->run_in_loop(std::bind(&TimerQueue::add_timer_in_loop, this, timer));
  return TimerID(timer, timer->sequence());
}
void TimerQueue::cancel(TimerID timer_id) {
  _loop->run_in_loop(std::bind(&TimerQueue::cancel_in_loop, this, timer_id));
}

void TimerQueue::add_timer_in_loop(Timer* timer) {
  _loop->assert_in_loop_thread();
  bool earlies_changed = insert(timer);

  if (earlies_changed) {
    reset_timer_fd(_timer_fd, timer->expertion_time());
  }
}
void TimerQueue::cancel_in_loop(TimerID timer_id) {
  _loop->assert_in_loop_thread();
  assert(_timers.size() == _active_timers.size());
  ActiveTimer timer(timer_id._timer, timer_id._sequence);
  ActiveTimerSet::iterator it = _active_timers.find(timer);
  if (it != _active_timers.end()) {
    size_t n = _timers.erase(Entry(it->first->expertion_time(), it->first));
    assert(n == 1);
    (void)n;
    delete it->first;  // FIXME: no delete please
    _active_timers.erase(it);
  } else if (_calling_expired_timers) {
    _canceling_timers.insert(timer);
  }
  assert(_timers.size() == _active_timers.size());
}

void TimerQueue::handle_read() {
  _loop->assert_in_loop_thread();
  Timestamp now(Timestamp::now());
  read_timer_fd(_timer_fd, now);

  std::vector<Entry> expired = get_expired(now);

  _calling_expired_timers = true;
  _canceling_timers.clear();
  for (const Entry& it : expired) it.second->run();
  _calling_expired_timers = false;
  reset(expired, now);
}

std::vector<TimerQueue::Entry> TimerQueue::get_expired(Timestamp now) {
  assert(_timers.size() == _active_timers.size());
  std::vector<Entry> expired;
  Entry sentry(now, reinterpret_cast<Timer*>(UINTPTR_MAX));  // TODO ?
  TimerList::iterator end = _timers.lower_bound(sentry);
  assert(end == _timers.end() || now < end->first);
  std::copy(_timers.begin(), end, back_inserter(expired));
  _timers.erase(_timers.begin(), end);

  for (const Entry& it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    size_t n = _active_timers.erase(timer);
    assert(n == 1);
    (void)n;
  }

  assert(_timers.size() == _active_timers.size());
  return expired;
}

void TimerQueue::reset(const std::vector<Entry>& expired, Timestamp now) {
  Timestamp next_expired;

  for (const Entry& it : expired) {
    ActiveTimer timer(it.second, it.second->sequence());
    if (it.second->repeated() &&
        _canceling_timers.find(timer) == _canceling_timers.end()) {
      it.second->restart(now);
      insert(it.second);
    } else {
      delete it.second;  // TODO no delete
    }
  }

  if (!_timers.empty()) {
    next_expired = _timers.begin()->second->expertion_time();
  }

  if (next_expired.valid()) {
    reset_timer_fd(_timer_fd, next_expired);
  }
}

bool TimerQueue::insert(Timer* timer) {
  _loop->assert_in_loop_thread();
  assert(_timers.size() == _active_timers.size());
  bool earliestChanged = false;
  Timestamp when = timer->expertion_time();
  TimerList::iterator it = _timers.begin();
  if (it == _timers.end() || when < it->first) {
    earliestChanged = true;
  }
  {
    std::pair<TimerList::iterator, bool> result =
        _timers.insert(Entry(when, timer));
    assert(result.second);
    (void)result;
  }
  {
    std::pair<ActiveTimerSet::iterator, bool> result =
        _active_timers.insert(ActiveTimer(timer, timer->sequence()));
    assert(result.second);
    (void)result;
  }

  assert(_timers.size() == _active_timers.size());
  return earliestChanged;
}