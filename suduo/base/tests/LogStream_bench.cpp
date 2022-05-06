#include <stdio.h>

#include <iostream>
#include <sstream>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/LogStream.h"
#include "suduo/base/Timestamp.h"
#define __STDC_FORMAT_MACROS
#include <inttypes.h>

using namespace suduo;

const size_t N = 1000000;

#pragma GCC diagnostic ignored "-Wold-style-cast"
const string test_string = "Hello 0123456789";
const string test_string2 = " abcdefghijklmnopqrstuvwxyz ";

char thread_time[64];
time_t thread_lastSecond;
void time_to_string(const Timestamp& now) {
  time_t seconds = now.get_seconds_in_int64();
  if (seconds != thread_lastSecond) {
    thread_lastSecond = seconds;
    struct tm tm_time;
    gmtime_r(&seconds, &tm_time);
    int len =
        snprintf(thread_time, 64, "%4d%02d%02d %02d:%02d:%02d",
                 tm_time.tm_year + 1900, tm_time.tm_mon + 1, tm_time.tm_mday,
                 tm_time.tm_hour, tm_time.tm_min, tm_time.tm_sec);
    assert(len == 17);
  }
}

template <typename T>
void benchPrintf(const char* fmt) {
  char buf[32];
  Timestamp start(Timestamp::now());
  for (size_t i = 0; i < N; ++i) snprintf(buf, sizeof buf, fmt, (T)(i));
  Timestamp end(Timestamp::now());

  printf("benchPrintf %f\n", (end - start).get_seconds_in_double());
}

template <typename T>
void benchStringStream() {
  Timestamp start(Timestamp::now());
  std::ostringstream os;

  for (size_t i = 0; i < N; ++i) {
    os << (T)(i);
    os.seekp(0, std::ios_base::beg);
  }
  Timestamp end(Timestamp::now());

  printf("benchStringStream %f\n", (end - start).get_seconds_in_double());
}

template <typename T>
void benchLogStream() {
  Timestamp start(Timestamp::now());

  LogStream os;
  for (size_t i = 0; i < N; ++i) {
    os << (T)(i);

    os.reset_buffer();
  }

  Timestamp end(Timestamp::now());

  printf("benchLogStream %f\n", (end - start).get_seconds_in_double());
}

template <typename T>
void benchIOStream() {
  Timestamp start(Timestamp::now());
  std::ostream os(std::cout.rdbuf());
  for (size_t i = 0; i < N; ++i) {
    os << (T)(i);
    os.seekp(0, std::ios_base::beg);
  }
  Timestamp end(Timestamp::now());

  printf("benchIOStream %f\n", (end - start).get_seconds_in_double());
}

void benchLogStream_string() {
  Timestamp start(Timestamp::now());
  LogStream os;
  for (size_t i = 0; i < N; ++i) {
    os << "Hello 0123456789"
       << " abcdefghijklmnopqrstuvwxyz " << 15;
    os.reset_buffer();
  }
  Timestamp end(Timestamp::now());

  printf("benchLogStream_string %f\n", (end - start).get_seconds_in_double());
}

void benchLogStream_now() {
  Timestamp start(Timestamp::now());
  LogStream os;
  for (size_t i = 0; i < N; ++i) {
    time_to_string(Timestamp::now());
    os << thread_time;
    os.reset_buffer();
    // struct timespec ts = {0, 1000 * 1000 * 1000 * 1000};
    // nanosleep(&ts, NULL);
  }
  Timestamp end(Timestamp::now());

  printf("benchLogStream_string %f\n", (end - start).get_seconds_in_double());
}

int main() {
  benchPrintf<int>("%d");

  puts("int");
  benchPrintf<int>("%d");
  benchStringStream<int>();
  benchLogStream<int>();
  // benchIOStream<int>();

  puts("double");
  benchPrintf<double>("%.12g");
  benchStringStream<double>();
  benchLogStream<double>();
  // benchIOStream<double>();

  puts("int64_t");
  benchPrintf<int64_t>("%" PRId64);
  benchStringStream<int64_t>();
  benchLogStream<int64_t>();
  // benchIOStream<int64_t>();

  puts("void*");
  benchPrintf<void*>("%p");
  benchStringStream<void*>();
  benchLogStream<void*>();
  benchIOStream<void*>();
  puts("string");
  benchLogStream_string();
  puts("now");
  benchLogStream_now();
}
