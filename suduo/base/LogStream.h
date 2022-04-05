#ifndef LOG_STREAM_H
#define LOG_STREAM_H
#include <array>
#include <charconv>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <string>
#include <system_error>

#include "suduo/base/Thread.h"
namespace suduo {
const int MIN_BUFFER_SIZE = 4000;
const int MAX_BUFFER_SIZE = 4000 * 1000;
template <int SIZE>
class StreamBuffer {
  using BufferType = std::array<char, SIZE>;

 public:
  StreamBuffer() : _current(_buffer.begin()) {}

  void append(std::string buf) {
    if (availability() > buf.size()) {
      std::move(buf.begin(), buf.end(), _current);
      _current += buf.size();  // TODO change later
    }
  }

  void append(const char* buf, size_t len) {
    if (availability() > len) {
      std::move(buf, buf + len, _current);
      _current += len;  // TODO change later
    }
  }

  const char* data() const { return _buffer.data(); }
  char* current_ptr() const { return _current; }
  char* end() { return _buffer.end(); }

  int size() const { return (_current - _buffer.begin()); }
  int availability() const { return (_buffer.end() - _current); }

  void move(int step) { _current += step; }  // TODO change later
  void reset() { _current = _buffer.begin(); }

  void be_zero() { _buffer.fill(0); }

  std::string to_string() const { return std::string(_buffer.data(), size()); }

 private:
  BufferType _buffer;
  char* _current;
};

class LogStream {
 public:
  using Buffer = suduo::StreamBuffer<MIN_BUFFER_SIZE>;
  using self = suduo::LogStream;
  self& operator<<(bool val) {
    _buffer.append(val ? "true" : "false");
    return *this;
  }

  self& operator<<(int val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != std::errc()) std::printf("error\n");
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(unsigned int val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != std::errc()) std::printf("error\n");
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(long val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != std::errc()) std::printf("error\n");
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(unsigned long val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != std::errc()) std::printf("error\n");
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(long long val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != std::errc()) std::printf("error\n");
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(unsigned long long val) {
    auto [ptr, ec] = std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    if (ec != std::errc()) std::printf("error\n");
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }

  self& operator<<(float val) {
    auto [ptr, ec] =
        std::to_chars(_buffer.current_ptr(), _buffer.end(), val,
                      std::chars_format::general, 12);  // TODO notsure
    if (ec != std::errc()) std::printf("error\n");
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }
  self& operator<<(double val) {
    auto [ptr, ec] =
        std::to_chars(_buffer.current_ptr(), _buffer.end(), val,
                      std::chars_format::general, 12);  // TODO notsure
    if (ec != std::errc()) std::printf("error\n");
    _buffer.move(ptr - _buffer.current_ptr());
    return *this;
  }

  self& operator<<(const void* val) {
    int len = snprintf(_buffer.current_ptr(), 42, "0x%X", val);
    _buffer.move(len);
    return *this;
  }

  self& operator<<(char val) {
    _buffer.append(&val, 1);
    return *this;
  }

  self& operator<<(const char* val) {
    if (val) {
      _buffer.append(val, std::strlen(val));
    }
    // printf("%X\n", _buffer.current_ptr());
    return *this;
  }
  self& operator<<(std::string&& val) {
    _buffer.append(std::move(val));
    return *this;
  }
  self& operator<<(std::string& val) {
    _buffer.append(val);
    return *this;
  }
  self& operator<<(const std::string& val) {
    _buffer.append(val);
    return *this;
  }
  self& operator<<(Buffer val) {
    *this << val.data();
    return *this;
  }

  void append(std::string& val) { _buffer.append(val); }
  void append(const char* val, int len) { _buffer.append(val, len); }

  const Buffer& buffer() { return _buffer; }
  void reset_buffer() { _buffer.reset(); }

 private:
  Buffer _buffer;
};

}  // namespace suduo

#endif