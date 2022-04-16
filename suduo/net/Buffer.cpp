#include "Buffer.h"

#include <bits/types/struct_iovec.h>
#include <sys/types.h>

#include <cerrno>
#include <cstddef>

#include "suduo/net/SocketOpt.h"
// namespace suduo

using Buffer = suduo::net::Buffer;
const size_t Buffer::init_size = 1024;
const size_t Buffer::pre_append_size = 8;

ssize_t Buffer::read_fd(int fd, int* saved_errno) {
  char extrabuf[65536];
  iovec vec[2];
  const size_t writeable = writeadble_bytes();
  vec[0].iov_base = begin() + write_index;
  vec[0].iov_len = writeable;
  vec[1].iov_base = extrabuf;
  vec[1].iov_len = sizeof(extrabuf);

  const int iovcnt = (writeable < sizeof(extrabuf)) ? 2 : 1;
  const ssize_t n = sockets::readv(fd, vec, iovcnt);
  if (n < 0) {
    *saved_errno = errno;
  } else if (static_cast<size_t>(n) <= writeable) {
    write_index += n;
  } else {
    write_index = _buffer.size();
    append(extrabuf, n - writeable);
  }
  return n;
}