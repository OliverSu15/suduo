#include "suduo/net2/EventLoop.h"

#include <signal.h>
#include <sys/eventfd.h>
#include <unistd.h>

#include "suduo/base/BlockingQueue.h"
#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Logger.h"
#include "suduo/net/TimerQueue.h"
#include "suduo/net2/Channel.h"
#include "suduo/net2/Poller.h"
#include "suduo/net2/SocketOpt.h"
using EventLoop = suduo::net::EventLoop;
using namespace suduo::net;

__thread EventLoop* loop_in_this_thrad = nullptr;

const int POLL_TIME_MS = 10000;

int create_event_fd() {
  int event_fd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
  if (event_fd < 0) {
    LOG_ERROR << "EventLoop:create_event_fd()";
  }
  return event_fd;
}

class IgnoreSigPipe {
 public:
  IgnoreSigPipe() { signal(SIGPIPE, SIG_IGN); }
};

IgnoreSigPipe inti_obj;

EventLoop* EventLoop::get_event_loop_of_current_thread() {
  return loop_in_this_thrad;
}

EventLoop::EventLoop()
    : _running(false),
      _event_handling(false),
      _calling_pending_functors(false),
      _iteratoin(0),
      _thread_ID(Current_thread_info::tid()),
      _poller(Poller::new_default_poller(this)),
      _timer_queue(new TimerQueue(this)),
      wakeup_fd(create_event_fd()),
      _wake_up_channel(new Channel(this, wakeup_fd)),
      _current_active_channel(nullptr) {
  LOG_DEBUG << "EventLoop created " << this << " in thread " << _thread_ID;

  if (loop_in_this_thrad) {
    LOG_FATAL << "Another EventLoop " << loop_in_this_thrad
              << " exists in this thread " << _thread_ID;
  } else {
    loop_in_this_thrad = this;
  }

  _wake_up_channel->set_read_callback(std::bind(&EventLoop::handle_read, this));
  _wake_up_channel->enable_reading();
}

EventLoop::~EventLoop() {
  LOG_DEBUG << "EventLoop " << this << " of thread " << _thread_ID
            << " destructs in thread " << Current_thread_info::tid();
  _wake_up_channel->disable_all();
  _wake_up_channel->remove();
  close(wakeup_fd);
  loop_in_this_thrad = nullptr;
}

void EventLoop::loop() {
  assert_in_loop_thread();
  _running = true;
  LOG_TRACE << "EventLoop " << this << " start looping";
  while (_running) {
    _active_channel.clear();
    _poll_return_time = _poller->poll(POLL_TIME_MS, &_active_channel);

    ++_iteratoin;

    if (Logger::log_level() <= Logger::LogLevel::TRACE) {
      print_active_channels();
    }
    // TODO sort channel by priority
    _event_handling = true;
    for (Channel* channel : _active_channel) {
      _current_active_channel = channel;
      _current_active_channel->handle_event(_poll_return_time);
    }

    _current_active_channel = nullptr;
    _event_handling = false;

    do_pending_functors();
  }
}

void EventLoop::quit() {
  _running = false;
  // There is a chance that loop() just executes while(!quit_) and exits,
  // then EventLoop destructs, then we are accessing an invalid object.
  // Can be fixed using mutex_ in both places.
  if (!is_in_loop_thread()) {
    wakeup();
  }
}

void EventLoop::run_in_loop(Functor cb) {
  if (is_in_loop_thread()) {
    cb();
  } else {
    queue_in_loop(std::move(cb));
  }
}

void EventLoop::queue_in_loop(Functor cb) {
  {
    MutexLockGuard lock(_mutex);
    _pending_functors.push_back(std::move(cb));
  }

  if (!is_in_loop_thread() || _calling_pending_functors) {
    wakeup();
  }
}

size_t EventLoop::queueSize() const {
  MutexLockGuard lock(_mutex);
  return _pending_functors.size();
}

TimerID EventLoop::run_at(Timestamp time, TimerCallback cb) {
  return _timer_queue->add_timer(std::move(cb), time, 0.0);
}

TimerID EventLoop::run_after(double delay, TimerCallback cb) {
  Timestamp time = Timestamp::now() + Timestamp::SecondsDouble(delay);
  return run_at(time, std::move(cb));
}

TimerID EventLoop::run_every(double interval, TimerCallback cb) {
  Timestamp time = Timestamp::now() + Timestamp::SecondsDouble(interval);
  return _timer_queue->add_timer(std::move(cb), time, interval);
}

void EventLoop::cancel(TimerID timer_id) {
  return _timer_queue->cancel(timer_id);
}

void EventLoop::wakeup() {
  uint64_t one = 1;
  ssize_t n = sockets::write(wakeup_fd, &one, sizeof(one));
  if (n != sizeof(one)) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n << " bytes instead of 8";
  }
}

void EventLoop::handle_read() {
  uint64_t one = 1;
  ssize_t n = sockets::read(wakeup_fd, &one, sizeof one);
  if (n != sizeof one) {
    LOG_ERROR << "EventLoop::handleRead() reads " << n << " bytes instead of 8";
  }
}

void EventLoop::do_pending_functors() {
  std::vector<Functor> functors;
  _calling_pending_functors = true;
  {
    MutexLockGuard lock(_mutex);
    functors.swap(_pending_functors);
  }
  for (const Functor& functor : functors) {
    functor();
  }
  _calling_pending_functors = false;
}

void EventLoop::print_active_channels() const {
  for (const Channel* channel : _active_channel) {
    LOG_TRACE << "{" << channel->revents_to_string() << "} ";
  }
}
