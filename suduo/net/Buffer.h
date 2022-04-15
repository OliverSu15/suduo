#ifndef BUFFER_H
#define BUFFER_H
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>

#include "suduo/net/Endian.h"
namespace suduo {
namespace net {
class Buffer {
 public:
  static const size_t init_size;
  static const size_t pre_append_size;

  explicit Buffer(size_t size = init_size)
      : _buffer(pre_append_size + init_size),
        read_index(pre_append_size),
        write_index(pre_append_size) {}

  const char* find_CRLF() const {
    const char* ptr = std::search(peek(), write_begin(), CRLF, CRLF + 2);
    return ptr == write_begin() ? nullptr : ptr;
  }
  const char* find_CRLF(const char* start) const {
    const char* ptr = std::search(start, write_begin(), CRLF, CRLF + 2);
    return ptr == write_begin() ? nullptr : ptr;
  }

  const char* find_EOL() const {
    const char* ptr = std::find(peek(), write_begin(), '\n');
    return ptr;
  }
  const char* find_EOL(const char* start) const {
    const char* ptr = std::find(start, write_begin(), '\n');
    return ptr;
  }

  void retrieve(size_t len) {
    if (len < readable_bytes()) {
      read_index += len;
    } else {
      retrieve_all();
    }
  }
  void retrieve_until(const char* end) { retrieve(end - peek()); }
  void retrieve_int_64() { retrieve(sizeof(int64_t)); }
  void retrieve_int_32() { retrieve(sizeof(int32_t)); }
  void retrieve_int_16() { retrieve(sizeof(int16_t)); }
  void retrieve_int_8() { retrieve(sizeof(int8_t)); }
  void retrieve_all() {
    read_index = pre_append_size;
    write_index = pre_append_size;
  }

  std::string retrieve_as_string(size_t len) {
    std::string result(peek(), len);
    retrieve(len);
    return result;
  }
  std::string retrieve_all_as_string() {
    return retrieve_as_string(readable_bytes());
  }

  void append(const std::string& str) { append(str.data(), str.size()); }
  void append(const char* data, size_t len) {
    ensure_writeable_bytes(len);
    std::copy(data, data + len, write_begin());
    has_written(len);
  }
  void append(const void* data, size_t len) {
    append(static_cast<const char*>(data), len);
  }

  void ensure_writeable_bytes(size_t len) {
    if (writeadble_bytes() < len) {
      make_space(len);
    }
  }

  void has_written(size_t len) { write_index += len; }

  void unwrite(size_t len) { write_index -= len; }

  void append_int_64(int64_t x) {
    int64_t data = sockets::host_to_network_64(x);
    append(&data, sizeof(data));
  }
  void append_int_32(int32_t x) {
    int32_t data = sockets::host_to_network_32(x);
    append(&data, sizeof(data));
  }
  void append_int_16(int16_t x) {
    int16_t data = sockets::host_to_network_16(x);
    append(&data, sizeof(data));
  }
  void append_int_8(int8_t x) { append(&x, sizeof(x)); }

  int64_t read_int_64() {
    int64_t result = peek_int_64();
    retrieve_int_64();
    return result;
  }
  int32_t read_int_32() {
    int32_t result = peek_int_32();
    retrieve_int_32();
    return result;
  }
  int16_t read_int_16() {
    int16_t result = peek_int_16();
    retrieve_int_16();
    return result;
  }
  int8_t read_int_8() {
    int8_t result = peek_int_8();
    retrieve_int_8();
    return result;
  }

  int64_t peek_int_64() const {
    int64_t data;
    std::memcpy(&data, peek(), sizeof(data));
    return sockets::network_to_host_64(data);
  }
  int32_t peek_int_32() const {
    int32_t data;
    std::memcpy(&data, peek(), sizeof(data));
    return sockets::network_to_host_32(data);
  }
  int16_t peek_int_16() const {
    int16_t data;
    std::memcpy(&data, peek(), sizeof(data));
    return sockets::network_to_host_16(data);
  }
  int8_t peek_int_8() const {
    int8_t data = *peek();
    return data;
  }

  void prepend_int_64(int64_t x) {
    int64_t data = sockets::host_to_network_64(x);
    prepend(&data, sizeof(data));
  }
  void prepend_int_32(int32_t x) {
    int32_t data = sockets::host_to_network_32(x);
    prepend(&data, sizeof(data));
  }
  void prepend_int_16(int16_t x) {
    int16_t data = sockets::host_to_network_16(x);
    prepend(&data, sizeof(data));
  }
  void prepend_int_8(int8_t x) { prepend(&x, sizeof(x)); }
  void prepend(const void* data, size_t len) {
    read_index -= len;
    const char* d = static_cast<const char*>(data);
    std::copy(d, d + len, begin() + read_index);
  }

  void shrink() { _buffer.shrink_to_fit(); }

  size_t internal_capacity() const { return _buffer.capacity(); }

  size_t readable_bytes() const { return write_index - read_index; }
  size_t writeadble_bytes() const { return _buffer.size() - write_index; }
  size_t pre_append_bytes() const { return read_index; }

  // TODO change name
  const char* peek() const { return begin() + read_index; }

  char* write_begin() { return begin() + write_index; }
  const char* write_begin() const { return begin() + write_index; }
  ssize_t read_fd(int fd, int* saved_errno);

 private:
  char* begin() { return &*_buffer.begin(); }
  const char* begin() const { return &*_buffer.begin(); }

  void make_space(size_t len) {
    if (writeadble_bytes() + pre_append_bytes() < len + pre_append_size) {
      _buffer.resize(write_index + len);
    } else {
      size_t readable = readable_bytes();
      std::copy(begin() + read_index, begin() + write_index,
                begin() + pre_append_size);
      read_index = pre_append_size;
      write_index = read_index + readable;
    }
  }

  std::vector<char> _buffer;
  size_t read_index;
  size_t write_index;

  constexpr static const char CRLF[] = "\r\n";
};
}  // namespace net
}  // namespace suduo
#endif