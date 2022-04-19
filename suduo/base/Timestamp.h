#ifndef TIMESTAMP_H
#define TIMESTAMP_H
#include <bits/types/struct_timespec.h>
#include <bits/types/time_t.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <ctime>
#include <iostream>
#include <iterator>
#include <ratio>
#include <string>
#include <type_traits>
#include <utility>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/copyable.h"
namespace suduo {
using string = std::string;
class Timestamp : copyable {
 public:
  using SystemClock = std::chrono::system_clock;

  using Hours = std::chrono::hours;
  using Mintues = std::chrono::minutes;
  using Seconds = std::chrono::seconds;
  using Milliseconds = std::chrono::milliseconds;
  using Microseconds = std::chrono::microseconds;
  using Nanoseconds = std::chrono::nanoseconds;

  using HoursDouble = std::chrono::duration<double, std::ratio<3600>>;
  using MintuesDouble = std::chrono::duration<double, std::ratio<60>>;
  using SecondsDouble = std::chrono::duration<double>;
  using MillisecondsDouble = std::chrono::duration<double, std::milli>;
  using MicrosecondsDouble = std::chrono::duration<double, std::micro>;
  // using NanosecondsDouble = std::chrono::duration<double, std::nano>;

  using HoursTimePoint = std::chrono::time_point<SystemClock, Hours>;
  using MinutesTimePoint = std::chrono::time_point<SystemClock, Mintues>;
  using SecondsTimePoint = std::chrono::time_point<SystemClock, Seconds>;
  using MsTimePoint = std::chrono::time_point<SystemClock, Milliseconds>;
  using UsTimePoint = std::chrono::time_point<SystemClock, Microseconds>;
  using NsTimePoint = std::chrono::time_point<SystemClock, Nanoseconds>;

  Timestamp() = default;
  Timestamp(const Timestamp& other) : _time_point(other._time_point) {}
  Timestamp(const Timestamp&& other) noexcept
      : _time_point(other._time_point) {}
  Timestamp(const NsTimePoint& new_time) : _time_point(new_time) {}
  Timestamp(const Nanoseconds& new_time) : _time_point(new_time) {}
  Timestamp(const MicrosecondsDouble& new_time)
      : _time_point(std::chrono::duration_cast<Nanoseconds>(new_time)) {}
  Timestamp(const time_t& new_time) : Timestamp(from_unix_time(new_time)) {}

  ~Timestamp() = default;

  void swap(Timestamp& that) { std::swap(that._time_point, _time_point); }

  inline const NsTimePoint& get_Time_Point() const { return _time_point; }
  inline Nanoseconds get_time_since_epoch() const {
    return _time_point.time_since_epoch();
  }

  static Timestamp now() { return {SystemClock::now()}; }
  static inline Timestamp from_unix_time(const time_t& time) {
    return {SystemClock::from_time_t(time)};
  }
  static inline time_t to_unix_time(const Timestamp& time) {
    return SystemClock::to_time_t(time.get_Time_Point());
  }
  static inline timespec to_time_spec(const Timestamp& time) {
    return {static_cast<time_t>(time.get_seconds_in_int64()),
            static_cast<long>(time.get_nanoseconds_in_int64() %
                              SECOND_TO_NANOSECOND)};
  }

  inline string format_string(const string& format) const {
    std::array<char, 20> str{};
    std::time_t tt_ = SystemClock::to_time_t(_time_point);
    std::tm* tm_ = std::localtime(&tt_);
    std::strftime(str.data(), 20, format.c_str(), tm_);
    return {str.data()};
  }
  inline string to_string() const { return format_string(_time_format_string); }
  inline string to_log_string() const {
    return format_string(_log_time_format_string);
  }

  // TODO need to come out a better way to do this
  inline bool valid() const {
    return _time_point.time_since_epoch() != Nanoseconds::zero();
  }
  static Timestamp get_invalid() { return {Nanoseconds::zero()}; }

  // operators
  void operator+=(const Nanoseconds& duration) { _time_point += duration; }
  void operator-=(const Nanoseconds& duration) { _time_point -= duration; }
  void operator+=(const MicrosecondsDouble& duration) {
    _time_point += Timestamp(duration).get_time_since_epoch();
  }
  void operator-=(const MicrosecondsDouble& duration) {
    _time_point -= Timestamp(duration).get_time_since_epoch();
  }

  Timestamp& operator=(const Timestamp& other) {
    if (this != &other) {
      _time_point = other._time_point;
    }
    return *this;
  }
  Timestamp& operator=(Timestamp&& other) noexcept {
    _time_point = other._time_point;
    return *this;
  }

  friend inline Timestamp operator+(const Timestamp& lhs,
                                    const Timestamp::Nanoseconds& rhs);
  friend inline Timestamp operator+(const Timestamp& lhs,
                                    const Timestamp::MicrosecondsDouble& rhs);
  friend inline Timestamp operator+(const Timestamp::Nanoseconds& lhs,
                                    const Timestamp& rhs);
  friend inline Timestamp operator+(const Timestamp::MicrosecondsDouble& lhs,
                                    const Timestamp& rhs);
  friend inline Timestamp operator-(const Timestamp& lhs,
                                    const Timestamp::Nanoseconds& rhs);
  friend inline Timestamp operator-(const Timestamp& lhs,
                                    const Timestamp::MicrosecondsDouble& rhs);
  friend inline Timestamp operator-(const Timestamp& lhs, const Timestamp& rhs);

  friend inline bool operator<(const Timestamp& lhs, const Timestamp& rhs);
  friend inline bool operator<=(const Timestamp& lhs, const Timestamp& rhs);
  friend inline bool operator>(const Timestamp& lhs, const Timestamp& rhs);
  friend inline bool operator>=(const Timestamp& lhs, const Timestamp& rhs);
  friend inline bool operator==(const Timestamp& lhs, const Timestamp& rhs);
  friend inline bool operator!=(const Timestamp& lhs, const Timestamp& rhs);

  inline int64_t get_hours_in_int64() const {
    return get_val_in_int64<Hours>();
  }
  inline int64_t get_minutes_in_int64() const {
    return get_val_in_int64<Mintues>();
  }
  inline int64_t get_seconds_in_int64() const {
    return get_val_in_int64<Seconds>();
  }
  inline int64_t get_milliseconds_in_int64() const {
    return get_val_in_int64<Milliseconds>();
  }
  inline int64_t get_microseconds_in_int64() const {
    return get_val_in_int64<Microseconds>();
  }
  inline int64_t get_nanoseconds_in_int64() const {
    return get_val_in_int64<Nanoseconds>();
  }

  inline double get_hours_in_double() const {
    return get_val_in_double<HoursDouble>();
  }
  inline double get_minutes_in_double() const {
    return get_val_in_double<MintuesDouble>();
  }
  inline double get_seconds_in_double() const {
    return get_val_in_double<SecondsDouble>();
  }
  inline double get_milliseconds_in_double() const {
    return get_val_in_double<MillisecondsDouble>();
  }
  inline double get_microseconds_in_double() const {
    return get_val_in_double<MicrosecondsDouble>();
  }

  template <typename ToDuration>
  inline double get_val_in_double() const {
    return ToDuration(_time_point.time_since_epoch()).count();
  }
  template <typename ToDuration>
  inline int64_t get_val_in_int64() const {
    return (std::chrono::duration_cast<ToDuration>(
                _time_point.time_since_epoch()))
        .count();
  }

 private:
  NsTimePoint _time_point;
  static const string _time_format_string;
  static const string _log_time_format_string;
};

// operators

inline Timestamp operator+(const Timestamp& lhs,
                           const Timestamp::Nanoseconds& rhs) {
  return {lhs._time_point + rhs};
}
inline Timestamp operator+(const Timestamp& lhs,
                           const Timestamp::MicrosecondsDouble& rhs) {
  return lhs + std::chrono::duration_cast<Timestamp::Nanoseconds>(rhs);
}
inline Timestamp operator+(const Timestamp::Nanoseconds& lhs,
                           const Timestamp& rhs) {
  return rhs + lhs;
}
inline Timestamp operator+(const Timestamp::MicrosecondsDouble& lhs,
                           const Timestamp& rhs) {
  return rhs + lhs;
}

inline Timestamp operator-(const Timestamp& lhs,
                           const Timestamp::Nanoseconds& rhs) {
  return {lhs._time_point - rhs};
}
inline Timestamp operator-(const Timestamp& lhs,
                           const Timestamp::MicrosecondsDouble& rhs) {
  return {lhs.get_time_since_epoch() - Timestamp(rhs).get_time_since_epoch()};
}
inline Timestamp operator-(const Timestamp& lhs, const Timestamp& rhs) {
  return {lhs.get_time_since_epoch() - rhs.get_time_since_epoch()};
}

inline bool operator<(const Timestamp& lhs, const Timestamp& rhs) {
  return lhs._time_point < rhs._time_point;
}
inline bool operator<=(const Timestamp& lhs, const Timestamp& rhs) {
  return lhs._time_point <= rhs._time_point;
}
inline bool operator>(const Timestamp& lhs, const Timestamp& rhs) {
  return lhs._time_point > rhs._time_point;
}
inline bool operator>=(const Timestamp& lhs, const Timestamp& rhs) {
  return lhs._time_point >= rhs._time_point;
}
inline bool operator==(const Timestamp& lhs, const Timestamp& rhs) {
  return lhs._time_point == rhs._time_point;
}
inline bool operator!=(const Timestamp& lhs, const Timestamp& rhs) {
  return !(lhs == rhs);
}

}  // namespace suduo
#endif