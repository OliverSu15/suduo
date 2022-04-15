#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include <sched.h>

#include <algorithm>
#include <any>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Timestamp.h"
#include "suduo/net/Callbacks.h"
#include "suduo/net/TimerId.h"
namespace suduo {
namespace net {
class Channel;
class Poller;
class TimerQueue;
class EventLoop : noncopyable {
 public:
  using Functor = std::function<void()>;

  EventLoop();
  ~EventLoop();

  void loop();
  void quit();

  Timestamp poll_return_time() const { return _poll_return_time; }
  int64_t iteration() const { return _iteratoin; }

  void run_in_loop(Functor cb);
  void queue_in_loop(Functor cb);

  size_t queueSize() const;

  TimerID run_at(Timestamp time, TimerCallback cb);
  TimerID run_after(double delay, TimerCallback cb);     // delay seconds
  TimerID run_every(double interval, TimerCallback cb);  // interval seconds

  void cancel(TimerID timer_id);

  void wakeup();
  void update_channel(Channel* channel);
  void remove_channel(Channel* channel);
  bool has_channel(Channel* channel);

  void assert_in_loop_thread() {
    if (!is_in_loop_thread()) {
      abort_not_in_loop_thread();
    }
  }

  bool is_in_loop_thread() const {
    return _thread_ID == Current_thread_info::tid();
  }

  bool event_handing() const { return _event_handling; }

  void set_context(const std::any& context) { _context = context; }
  const std::any& get_context() const { return _context; }
  const std::any* get_mutable_context() { return &_context; }

  static EventLoop* get_event_loop_of_current_thread();

 private:
  void abort_not_in_loop_thread();
  void handle_read();
  void do_pending_functors();
  void print_active_channels() const;

  using ChannelList = std::vector<Channel*>;

  bool _looping;  // TODO atomic
  std::atomic_bool _quit;
  bool _event_handling;            // TODO atomic
  bool _calling_pending_functors;  // TODO atomic
  int64_t _iteratoin;
  const pid_t _thread_ID;
  Timestamp _poll_return_time;
  std::unique_ptr<Poller> _poller;
  std::unique_ptr<TimerQueue> _timer_queue;
  int wakeup_fd;

  std::unique_ptr<Channel> _wake_up_channel;
  std::any _context;

  ChannelList _active_channel;
  Channel* _current_active_channel;

  mutable MutexLock _mutex;
  std::vector<Functor> _pending_functors;
};
}  // namespace net
}  // namespace suduo
#endif