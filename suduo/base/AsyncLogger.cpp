#include "AsyncLogger.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <utility>

#include "suduo/base/LogFile.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Timestamp.h"
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
      _current_buffer(new Buffer),
      _next_buffer(new Buffer),
      _buffer_poll() {
  //_current_buffer->be_zero();
  //_next_buffer->be_zero();
  _buffer_poll.reserve(Buffer_Poll_Size);
}

void AsyncLogger::append(const char* log, size_t len) {
  MutexLockGuard lock(_mutex);
  if (_current_buffer->availability() > len) {
    _current_buffer->append(log,
                            len);  // TODO buffer的append speed is quite close
  } else {
    suduo::Timestamp start = suduo::Timestamp::now();
    _buffer_poll.push_back(std::move(_current_buffer));
    if (_next_buffer) {
      _current_buffer = std::move(_next_buffer);
    } else {
      _current_buffer.reset(new Buffer);
    }
    _current_buffer->append(log, len);
    suduo::Timestamp end = suduo::Timestamp::now();
    printf("~ %f\n", (end - start).get_microseconds_in_double());
    _condition.notify();
  }
}

void AsyncLogger::thread_func() {
  _latch.count_down();
  LogFile log_file(_filename, _roll_size, false);
  BufferPtr new_buffer_1(new Buffer);
  BufferPtr new_buffer_2(new Buffer);
  // new_buffer_1->be_zero();
  // new_buffer_2->be_zero();
  BufferPoll buffer_to_write;
  buffer_to_write.reserve(Buffer_Poll_Size);
  while (_running) {
    {
      MutexLockGuard lock(_mutex);
      if (_buffer_poll.empty()) {
        _condition.time_wait(Timestamp::Seconds(_flush_interval) +
                             Timestamp::now());
      }

      _buffer_poll.push_back(std::move(_current_buffer));
      _current_buffer = std::move(new_buffer_1);
      buffer_to_write.swap(_buffer_poll);

      if (!_next_buffer) {
        _next_buffer = std::move(new_buffer_2);
      }
    }
    if (buffer_to_write.size() > 25) {
      char buf[256];
      snprintf(
          buf, sizeof buf, "Dropped log messages at %s, %zd larger buffers\n",
          Timestamp::now().to_string().c_str(), buffer_to_write.size() - 2);
      fputs(buf, stderr);
      log_file.append(buf, static_cast<int>(strlen(buf)));
      buffer_to_write.erase(buffer_to_write.begin() + 2, buffer_to_write.end());
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
