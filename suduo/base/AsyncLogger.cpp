#include "AsyncLogger.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <utility>

#include "suduo/base/LogFile.h"
#include "suduo/base/Mutex.h"
using AsyncLogger = suduo::AsyncLogger;

AsyncLogger::AsyncLogger(const std::string& filename, int64_t roll_size,
                         int flush_interval)
    : _flush_interval(flush_interval),
      _roll_size(roll_size),
      _filename(filename),
      _running(false),
      _mutex(),
      _thread(std::bind(&AsyncLogger::thread_func, this), "AsyncLogger"),
      _latch(1),
      _condition(_mutex), /*not sure is safe*/
      current_buffer(new Buffer),
      next_buffer(new Buffer),
      _buffer_poll() {
  current_buffer->be_zero();
  next_buffer->be_zero();
  _buffer_poll.reserve(Buffer_Poll_Size);
}

void AsyncLogger::append(const char* log, size_t len) {
  MutexLockGuard lock(_mutex);
  if (current_buffer->availability() > len) {
    current_buffer->append(log, len);
  } else {
    _buffer_poll.push_back(std::move(current_buffer));
    if (next_buffer) {
      current_buffer = std::move(next_buffer);
    } else {
      current_buffer.reset(new Buffer);
    }
    current_buffer->append(log, len);
    _condition.notify();
  }
}

void AsyncLogger::thread_func() {
  _latch.count_down();
  LogFile log_file(_filename, _roll_size, false);
  BufferPtr new_buffer_1(new Buffer);
  BufferPtr new_buffer_2(new Buffer);
  new_buffer_1->be_zero();
  new_buffer_2->be_zero();
  BufferPoll buffer_to_write;
  buffer_to_write.reserve(16);
  while (_running) {
    MutexLockGuard lock(_mutex);
    if (_buffer_poll.empty()) {
      _condition.time_wait(std::chrono::seconds(_flush_interval));
    }

    _buffer_poll.push_back(std::move(current_buffer));
    current_buffer = std::move(new_buffer_1);
    buffer_to_write.swap(_buffer_poll);

    if (!next_buffer) {
      next_buffer = std::move(new_buffer_2);
    }

    for (const auto& i : buffer_to_write) {
      log_file.append(i->data(), i->size());
    }

    if (buffer_to_write.size() > 2) {
      buffer_to_write.resize(2);
    }

    if (!new_buffer_1) {
      new_buffer_1 = std::move(buffer_to_write.back());
      buffer_to_write.pop_back();
      new_buffer_1->reset();
    }

    if (!new_buffer_2) {
      new_buffer_2 = std::move(buffer_to_write.back());
      buffer_to_write.pop_back();
      new_buffer_2->reset();
    }

    buffer_to_write.clear();
    log_file.flush();
  }
  log_file.flush();
}
