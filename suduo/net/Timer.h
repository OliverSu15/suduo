#ifndef TIMER_H
#define TIMER_H
#include <atomic>
#include <cstdint>
#include <functional>
#include <utility>

#include "suduo/base/Timestamp.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
using TimerCallback = std::function<void()>;
namespace net {
class Timer : noncopyable {
 public:
  Timer(TimerCallback cb, Timestamp expertion_time, double interval = 0)
      : _callback(std::move(cb)),
        _expertion_time(expertion_time),
        _interval(interval),
        _repeated(interval > 0),
        _sequence(_created.fetch_add(1)) {}

  void run() { _callback(); }

  Timestamp expertion_time() const { return _expertion_time; }
  bool repeated() const { return _repeated; }
  int64_t sequence() const { return _sequence; }

  void restart(Timestamp now);

  static int64_t created() { return _created; }

 private:
  const TimerCallback _callback;
  Timestamp _expertion_time;
  const bool _repeated;
  const double _interval;
  const int64_t _sequence;

  static std::atomic_int64_t _created;
};
}  // namespace net
}  // namespace suduo
#endif