#include "FileUtil.h"

#include <bits/types/FILE.h>
#include <unistd.h>

#include <cstddef>
#include <cstdio>
#include <cstring>
using FileAppender = suduo::FileUtil::FileAppender;

FileAppender::FileAppender(const std::string& filename)
    : _fp(fopen(filename.c_str(), "ae")), _written_bytes(0), _buffer() {
  setbuf(_fp, _buffer.data());
}
FileAppender::FileAppender(const char* filename)
    : _fp(fopen(filename, "ae")), _written_bytes(0), _buffer() {
  setbuf(_fp, _buffer.data());
}

FileAppender::~FileAppender() { std::fclose(_fp); }

void FileAppender::append(const char* log, size_t len) {
  size_t written = 0;
  while (written != len) {
    size_t remain = len - written;
    size_t n = write(log + written, remain);

    if (n != remain) {
      int err = ferror(_fp);
      if (err) {
        fprintf(stderr, "FileAppender::append() failed %s\n", strerror(err));
        break;
      }
    }
    written += n;
  }
  _written_bytes += written;
}

void FileAppender::flush() { std::fflush(_fp); }

size_t FileAppender::write(const char* log, size_t len) {
  return fwrite_unlocked(log, 1, len, _fp);
}