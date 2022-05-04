#include "TimerQueue.h"

#include <bits/types/struct_FILE.h>
#include <sys/timerfd.h>

#include <cstdint>
#include <memory>

#include "suduo/base/Logger.h"
#include "suduo/base/Timestamp.h"
#include "suduo/net2/Channel.h"
#include "suduo/net2/EventLoop.h"
#include "suduo/net2/Timer.h"
using namespace suduo;
using suduo::net::Timer;

int create_timer_fd() {
  int timer_fd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  if (timer_fd < 0) {
    LOG_SYSFATAL << "Failed in timerfd_create";
  }
  return timer_fd;
}

timespec how_much_time_from_now(const Timestamp& when) {
  Timestamp diff = when - Timestamp::now();
  if (diff.get_microseconds_in_int64() < 100)
    diff = Timestamp(Timestamp::Microseconds(100));
  return Timestamp::to_time_spec(diff);
}

void read_timer_fd(int timer_fd, const Timestamp& now) {
  uint64_t how_many;
  ssize_t n = read(timer_fd, &how_many, sizeof(how_many));
  LOG_TRACE << "TimerQueue::handleRead() " << how_many << " at "
            << now.to_string();
  if (n != sizeof(how_many)) {
    LOG_ERROR << "TimerQueue::handleRead() reads " << n
              << " bytes instead of 8";
  }
}

void reset_timer_fd(int timer_fd, const Timestamp& expiration) {
  itimerspec new_value;
  itimerspec old_value;
  memZero(&new_value, sizeof(new_value));
  memZero(&old_value, sizeof(old_value));
  new_value.it_value = how_much_time_from_now(expiration);
  int ret = timerfd_settime(timer_fd, 0, &new_value, &old_value);
  if (ret) {
    LOG_SYSERR << "timerfd_settime()";
  }
}

using TimerQueue = suduo::net::TimerQueue;

TimerQueue::TimerQueue(EventLoop* loop)
    : _loop(loop),
      _timer_fd(create_timer_fd()),
      _timer_fd_channel(loop->poller(), _timer_fd),
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
}

uint64_t TimerQueue::add_timer(TimerCallback cb, const Timestamp& when,
                               double interval) {
  // Timer* timer = new Timer(std::move(cb), when, interval);
  std::unique_ptr<Timer> timer =
      std::make_unique<Timer>(std::move(cb), when, interval);

  uint64_t id = timer->id();
  Timestamp expertion_time = timer->expertion_time();

  bool earlies_changed = insert(std::move(timer));

  if (earlies_changed) {
    reset_timer_fd(_timer_fd, expertion_time);
  }
  return id;
}

void TimerQueue::cancel(uint64_t timer_id) {
  auto it = _active_timers.find(timer_id);
  if (it != _active_timers.end()) {
    // auto timers_it = _timers.find({it->second->expertion_time(), it->first});
    size_t n = _timers.erase({it->second->expertion_time(), it->first});
    assert(n == 1);
    _active_timers.erase(it);
  } else if (_calling_expired_timers) {
    _canceling_timers.insert(timer_id);
  }
  assert(_timers.size() == _active_timers.size());
}

bool TimerQueue::insert(std::unique_ptr<Timer> timer) {
  _loop->assert_in_loop_thread();

  assert(_timers.size() == _active_timers.size());

  bool earliestChanged = false;
  const Timestamp& when = timer->expertion_time();

  auto it = _timers.begin();
  if (it == _timers.end() || when < it->first.first) {
    earliestChanged = true;
  }

  std::pair<TimerMap::iterator, bool> entry_result =
      _timers.insert({{when, timer->id()}, std::move(timer)});
  assert(entry_result.second);

  const std::unique_ptr<Timer>& entry_timer = entry_result.first->second;

  std::pair<ActiveTimerMap::iterator, bool> active_timers_result =
      _active_timers.insert({entry_timer->id(), entry_timer});
  assert(active_timers_result.second);

  // assert(_timers.size() == _active_timers.size());
  return earliestChanged;
}

void TimerQueue::handle_read() {
  _loop->assert_in_loop_thread();
  Timestamp now = Timestamp::now();

  read_timer_fd(_timer_fd, now);

  // std::vector<Entry> expired = get_expired(now);
  auto iter = _timers.lower_bound({now, UINT64_MAX});
  std::vector<std::unique_ptr<Timer>> expired;
  if (iter != _timers.end()) {
    _calling_expired_timers = true;
    _canceling_timers.clear();
    auto ptr = _timers.begin();
    while (ptr != iter) {
      ptr->second->run();
      _active_timers.erase(ptr->first.second);
      expired.push_back(std::move(ptr->second));
      ptr++;
    }
    _timers.erase(_timers.begin(), iter);
    _calling_expired_timers = false;
  }

  reset(expired, now);
}

void TimerQueue::reset(std::vector<std::unique_ptr<Timer>>& expired,
                       Timestamp now) {
  Timestamp next_expired;

  for (auto& it : expired) {
    if (it->repeated() &&
        (_canceling_timers.find(it->id()) == _canceling_timers.end())) {
      it->restart(now);
      insert(std::move(it));
    }
  }

  if (!_timers.empty()) {
    next_expired = _timers.begin()->second->expertion_time();
  }

  if (next_expired.valid()) {
    reset_timer_fd(_timer_fd, next_expired);
  }
}