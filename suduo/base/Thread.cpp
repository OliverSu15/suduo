#include "Thread.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <pthread.h>

#include <cassert>
#include <cstdio>
#include <ctime>
#include <exception>
#include <functional>
#include <iostream>
#include <memory>

#include "suduo/base/CountDownLatch.h"
#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Logger.h"
namespace suduo {
namespace _detail {
using ThreadFunc = std::function<void()>;
void run_thread_func(ThreadFunc& _func, pthread_t* _thread_id, pid_t* tid,
                     CountDownLatch* latch, std::string& _name) {
  *tid = Current_thread_info::tid();
  tid = nullptr;
  latch->count_down();
  latch = nullptr;
  suduo::Current_thread_info::thread_name = _name.c_str();
  pthread_setname_np(*_thread_id, _name.c_str());

  try {
    _func();
  } catch (const Exception& e) {
    LOG_FATAL << "exception caught in Thread " << _name << "\n"
              << "reason:" << e.what() << "\n"
              << "stack trace:" << e.stackTrace() << "\n";
  } catch (const std::exception& e) {
    LOG_FATAL << "exception caught in Thread " << _name << "\n"
              << "reason:" << e.what() << "\n";
  } catch (...) {
    LOG_ERROR << "unknown exception caught in Thread" << _name;
    throw;
  }
}
void* run(void* it) {
  std::shared_ptr<ThreadFunc> ptr(static_cast<ThreadFunc*>(it));
  (*ptr)();
  return nullptr;
}
}  // namespace _detail
}  // namespace suduo
using Thread = suduo::Thread;

std::atomic_int32_t Thread::_thread_created;

Thread::Thread(ThreadFunc func_, const string& name)
    : _started(false),
      _joined(false),
      _thread_id(0),
      _tid(0),
      _func(std::move(func_)),
      _name(name),
      _latch(1) {
  _thread_created++;
}

Thread::~Thread() {
  if (_started && !_joined) pthread_detach(_thread_id);
}

void Thread::start() {
  assert(!_started);
  _started = true;
  auto* pfunc = new ThreadFunc(std::bind(suduo::_detail::run_thread_func, _func,
                                         &_thread_id, &_tid, &_latch, _name));
  int err = pthread_create(&_thread_id, nullptr, &suduo::_detail::run, pfunc);
  if (err) {
    _started = false;
    delete pfunc;
    LOG_SYSFATAL << "Failed in pthread_create";
  } else {
    _latch.wait();
    assert(_tid > 0);
    LOG_DEBUG << "Thread create success";
  }
}

void Thread::join() {
  assert(_started);
  assert(!_joined);
  _joined = true;
  ERROR_CHECK(pthread_join(_thread_id, nullptr));
}
