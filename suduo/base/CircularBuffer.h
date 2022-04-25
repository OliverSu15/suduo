#ifndef BLOCKING_QUEUE_H
#define BLOCKING_QUEUE_H

#include <vector>

#include "suduo/base/noncopyable.h"
namespace suduo {
namespace detail {
template <typename T>
class CircularBuffer : noncopyable {
  using BufferType = std::vector<T>;

 public:
  explicit CircularBuffer(int max_size)
      : _begin(0), _end(0), _size(0), _buffer(max_size) {
    _buffer.shrink_to_fit();  // In case allocate more memory than we need
  }

  void push(const T& val) {
    if (full()) return;  // TODO throw an exception ?
    _buffer[_end++] = val;
    if (_end >= _buffer.size()) _end = 0;
    _size++;
  }
  // like a STL container, pop on an empty buffer will cause unknown behavior
  T pop() {
    T val = _buffer[_begin++];
    if (_begin >= _buffer.size()) _begin = 0;
    _size--;
    return val;
  }

  bool full() const { return _size == _buffer.size(); }
  bool empty() const { return _size == 0; }
  int size() const { return _size; }
  int capacity() const { return _buffer.size() - size(); }

 private:
  BufferType _buffer;
  int _begin;
  int _end;
  int _size;
};
}  // namespace detail
}  // namespace suduo

#endif