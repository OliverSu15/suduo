#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#include <bits/types/time_t.h>

#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>
#include <type_traits>
namespace suduo {
class Timestamp {
  using SystemClock = std::chrono::system_clock;
  using TimePoint = std::chrono::time_point<SystemClock>;
  using Seconds = std::chrono::seconds;
  using Duration = std::chrono::duration<int64_t>;
  using string = std::string;

 public:
  Timestamp() = default;
  explicit Timestamp(int64_t new_time) : _time_point(Seconds(new_time)) {}
  explicit Timestamp(TimePoint& new_time) : _time_point(new_time) {}
  explicit Timestamp(TimePoint&& new_time) : _time_point(new_time) {}
  explicit Timestamp(Duration& new_time) : _time_point(new_time) {}
  explicit Timestamp(Duration&& new_time) : _time_point(new_time) {}

  void swap(Timestamp& that) { std::swap(that._time_point, _time_point); }

  const TimePoint& get_Time_Point() { return _time_point; }

  static inline Timestamp now() { return Timestamp(SystemClock::now()); }
  static inline Timestamp from_unix_time(time_t time) {
    return Timestamp(SystemClock::from_time_t(time));
  }

  // TODO 更改变量名甚至是实现
  //实现的并不好
  string to_string() {
    char str[20];
    std::time_t tt_ = SystemClock::to_time_t(_time_point);
    std::tm* tm_ = std::localtime(&tt_);
    std::strftime(str, 20, _time_format_string, tm_);
    return {str};
  }

 private:
  TimePoint _time_point;
  static const char* const _time_format_string;
};

// exits only to use the muduo test more easily
// TODO remove
inline double timeDifference(Timestamp high, Timestamp low) {
  auto diff = high.get_Time_Point() - low.get_Time_Point();
  return diff.count();  // nanosecond
}
}  // namespace suduo
#endif