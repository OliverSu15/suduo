#ifndef LOG_STREAM_H
#define LOG_STREAM_H
#include <array>
#include <charconv>
#include <cstddef>
#include <cstring>
#include <string>

#include "suduo/base/Thread.h"
namespace suduo {
const int MIN_BUFFER_SIZE = 1000;
template <int SIZE>
class StreamBuffer {
  using BufferType = std::array<char, SIZE>;

 public:
  StreamBuffer() : _current(_buffer.begin()) {}

  void append(std::string buf) {
    if (availability() >= buf.size()) {
      std::move(buf.begin(), buf.end(), _current);
      _current += buf.size();  // TODO change later
    }
  }

  void append(const char* buf, size_t len) {
    if (availability() >= len) {
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

  std::string to_string() { return std::string(_buffer.data(), size()); }

 private:
  BufferType _buffer;
  char* _current;
};

class LogStream {
  using Buffer = suduo::StreamBuffer<MIN_BUFFER_SIZE>;
  using self = suduo::LogStream;

 public:
  self& operator<<(bool val) {
    _buffer.append(val ? "true" : "false");
    return *this;
  }

  self& operator<<(int val) {
    std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    return *this;
  }
  self& operator<<(unsigned int val) {
    std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    return *this;
  }
  self& operator<<(long val) {
    std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    return *this;
  }
  self& operator<<(unsigned long val) {
    std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    return *this;
  }
  self& operator<<(long long val) {
    std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    return *this;
  }
  self& operator<<(unsigned long long val) {
    std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    return *this;
  }

  self& operator<<(float val) {
    std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    return *this;
  }
  self& operator<<(double val) {
    std::to_chars(_buffer.current_ptr(), _buffer.end(), val);
    return *this;
  }

  self& operator<<(char val) {
    _buffer.append(&val, 1);
    return *this;
  }

  self& operator<<(const char* val) {
    _buffer.append(val, std::strlen(val));
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