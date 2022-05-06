#ifndef ASYNC_LOGGER_H
#define ASYNC_LOGGER_H
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "suduo/base/Condition.h"
#include "suduo/base/CountDownLatch.h"
#include "suduo/base/LogStream.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Thread.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
class AsyncLogger : noncopyable {
 public:
  AsyncLogger(const std::string& filename, int64_t roll_size,
              int flush_interval = 3);
  ~AsyncLogger() {
    if (_running) {
      stop();
    }
  }

  void append(const char* log, size_t len);

  void start() {
    _running = true;
    _thread.start();
    _latch.wait();  // wait until thread start
  }

  void stop() {
    _running = false;
    _condition.notify();
    _thread.join();
  }

 private:
  void thread_func();
  using Buffer = StreamBuffer<MAX_BUFFER_SIZE>;
  using BufferPtr = std::unique_ptr<Buffer>;
  using BufferPoll = std::vector<BufferPtr>;

  static const int Buffer_Poll_Size = 16;

  const int _flush_interval;
  std::atomic_bool _running;
  const std::string _filename;
  const int64_t _roll_size;

  MutexLock _mutex;
  Thread _thread;
  CountDownLatch _latch;
  Condition _condition;

  BufferPtr _current_buffer;
  BufferPtr _next_buffer;
  BufferPoll _buffer_poll;
};
}  // namespace suduo

#endif