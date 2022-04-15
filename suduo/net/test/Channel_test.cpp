#include "suduo/net/Channel.h"

#include <stdio.h>
#include <sys/timerfd.h>
#include <unistd.h>

#include <functional>
#include <map>

#include "suduo/base/Logger.h"
#include "suduo/net/EventLoop.h"

using namespace suduo;
using namespace suduo::net;

void print(const char* msg) {
  static std::map<const char*, Timestamp> lasts;
  Timestamp& last = lasts[msg];
  Timestamp now = Timestamp::now();
  printf("%s tid %d %s delay %f\n", now.to_string().c_str(),
         Current_thread_info::tid(), msg, timeDifference(now, last));
  last = now;
}

namespace suduo {
namespace net {
namespace detail {
int create_timer_fd();
void read_timer_fd(int timerfd, Timestamp now);
}  // namespace detail
}  // namespace net
}  // namespace suduo

// Use relative time, immunized to wall clock changes.
class PeriodicTimer {
 public:
  PeriodicTimer(EventLoop* loop, double interval, const TimerCallback& cb)
      : loop_(loop),
        timerfd_(suduo::net::detail::create_timer_fd()),
        timerfdChannel_(loop, timerfd_),
        interval_(interval),
        cb_(cb) {
    timerfdChannel_.set_read_callback(
        std::bind(&PeriodicTimer::handleRead, this));
    timerfdChannel_.enable_reading();
  }

  void start() {
    struct itimerspec spec;
    // memZero(&spec, sizeof spec);
    spec.it_interval = toTimeSpec(interval_);
    spec.it_value = spec.it_interval;
    int ret = ::timerfd_settime(timerfd_, 0 /* relative timer */, &spec, NULL);
    if (ret) {
      LOG_SYSERR << "timerfd_settime()";
    }
  }

  ~PeriodicTimer() {
    timerfdChannel_.disable_all();
    timerfdChannel_.remove();
    ::close(timerfd_);
  }

 private:
  void handleRead() {
    loop_->assert_in_loop_thread();
    suduo::net::detail::read_timer_fd(timerfd_, Timestamp::now());
    if (cb_) cb_();
  }

  static struct timespec toTimeSpec(double seconds) {
    struct timespec ts;
    // memZero(&ts, sizeof ts);
    const int64_t kNanoSecondsPerSecond = 1000000000;
    const int kMinInterval = 100000;
    int64_t nanoseconds = static_cast<int64_t>(seconds * kNanoSecondsPerSecond);
    if (nanoseconds < kMinInterval) nanoseconds = kMinInterval;
    ts.tv_sec = static_cast<time_t>(nanoseconds / kNanoSecondsPerSecond);
    ts.tv_nsec = static_cast<long>(nanoseconds % kNanoSecondsPerSecond);
    return ts;
  }

  EventLoop* loop_;
  const int timerfd_;
  Channel timerfdChannel_;
  const double interval_;  // in seconds
  TimerCallback cb_;
};

int main(int argc, char* argv[]) {
  LOG_INFO << "pid = " << getpid() << ", tid = " << Current_thread_info::tid()
           << " Try adjusting the wall clock, see what happens.";
  EventLoop loop;
  PeriodicTimer timer(&loop, 1, std::bind(print, "PeriodicTimer"));
  timer.start();
  loop.run_every(1, std::bind(print, "EventLoop::runEvery"));
  loop.loop();
}
