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
      : begin(0), end(0), _size(0), buffer(max_size) {
    buffer.shrink_to_fit();  // In case allocate more memory than we need
  }

  void push(const T& val) {
    if (full()) return;  // TODO throw an exception ?
    buffer[end++] = val;
    if (end >= buffer.size()) end = 0;
    _size++;
  }
  // like a STL container, pop on an empty buffer will cause unknown behavior
  T pop() {
    T val = buffer[begin++];
    if (begin >= buffer.size()) begin = 0;
    _size--;
    return val;
  }

  bool full() const { return _size == buffer.size(); }
  bool empty() const { return _size == 0; }
  int size() const { return _size; }
  int capacity() const { return buffer.size() - size(); }

 private:
  BufferType buffer;
  int begin;
  int end;
  int _size;
};
}  // namespace detail
}  // namespace suduo

#endif