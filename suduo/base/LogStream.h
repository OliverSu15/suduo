#ifndef LOG_STREAM_H
#define LOG_STREAM_H
#include <algorithm>
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <sstream>
#include <string>
#include <system_error>

#include "suduo/base/Thread.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
inline void memzero(void* p, size_t n) { memset(p, 0, n); }
const int MIN_BUFFER_SIZE = 4000;
const int MAX_BUFFER_SIZE = 4000 * 1000;

template <int SIZE>
class StreamBuffer : noncopyable {
 public:
  StreamBuffer() : _current(_buffer), _end(_buffer + sizeof _buffer) {}

  inline void append(const char* buf, size_t len) {
    if (availability() > len) {
      memcpy(_current, buf, len);
      _current += len;
    }
  }

  inline const char* data() const { return _buffer; }
  inline char* current_ptr() { return (_current); }
  inline size_t size() const { return (_current - _buffer); }
  inline size_t availability() { return (_end - _current); }
  inline void move(size_t step) { _current += step; }
  inline void reset() { _current = _buffer; }

  string to_string() const { return {_buffer, size()}; }
  char* end() { return _end; }

 private:
  char _buffer[SIZE];
  char* _current;
  char* _end;
};

class LogStream : noncopyable {
 public:
  using Buffer = suduo::StreamBuffer<MIN_BUFFER_SIZE>;
  using self = suduo::LogStream;
  self& operator<<(bool val) {
    _buffer.append(val ? "true" : "false", val ? 4 : 5);
    return *this;
  }

  self& operator<<(int val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);

    if (ec != err) std::printf("error\n");  // FIXME change the output

    _buffer.move(ptr - _buffer.current_ptr());

    return *this;
  }
  self& operator<<(unsigned int val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != err) std::printf("error\n");  // FIXME change the output
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(long val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != err) std::printf("error\n");  // FIXME change the output
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(unsigned long val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != err) std::printf("error\n");  // FIXME change the output
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(long long val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != err) std::printf("error\n");  // FIXME change the output
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(unsigned long long val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != err) std::printf("error\n");  // FIXME change the output
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }

  self& operator<<(float val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val,
                                   std::chars_format::general, 12);
    if (ec != err) std::printf("error\n");  // FIXME change the output
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(double val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val,
                                   std::chars_format::general, 12);
    if (ec != err) std::printf("error\n");  // FIXME change the output
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }

  self& operator<<(const void* val) {
    char* buf = _buffer.current_ptr();
    buf[0] = '0';
    buf[1] = 'x';
    auto [ptr, ec] = std::to_chars(buf + 2, _buffer.end(),
                                   reinterpret_cast<size_t>(val), 16);
    if (ec != err) std::printf("error\n");  // FIXME change the output
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }

  self& operator<<(char val) {
    _buffer.append(&val, 1);
    return *this;
  }

  self& operator<<(const char* val) {
    if (val) {
      _buffer.append(val, strlen(val));
    }
    return *this;
  }
  self& operator<<(const string& val) {
    _buffer.append(val.c_str(), val.size());
    return *this;
  }
  self& operator<<(Buffer val) {
    *this << val.data();
    return *this;
  }

  // inline void append(const string& val) { _buffer.append(val); }
  inline void append(const char* val, int len) { _buffer.append(val, len); }

  inline const Buffer& buffer() const { return _buffer; }
  inline void reset_buffer() { _buffer.reset(); }

 private:
  // std::ostringstream stream;
  Buffer _buffer;
  static const std::errc err;
};

}  // namespace suduo

#endif