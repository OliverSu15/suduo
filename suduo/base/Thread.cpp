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
namespace suduo {
namespace _detail {
using Func = std::function<void()>;
void run_thread_func(Func& _func, pthread_t* _thread_id, pid_t* tid,
                     CountDownLatch* latch, std::string& _name) {
  *tid = Current_thread_info::tid();
  tid = nullptr;
  latch->count_down();
  latch = nullptr;
  // printf("%u run to here\n", *_thread_id);
  suduo::Current_thread_info::thread_name = _name.c_str();
  pthread_setname_np(*_thread_id, _name.c_str());

  try {
    _func();
  } catch (const std::exception& e) {
    // TODO LOG exception info
  } catch (...) {
    // TODO LOG exception info
    throw;
  }
}
void* run(void* it) {
  Func* ptr = static_cast<Func*>(it);
  (*ptr)();
  delete ptr;
  return nullptr;
}
}  // namespace _detail
}  // namespace suduo
using Thread = suduo::Thread;

std::atomic_int32_t Thread::numCreated_;

Thread::Thread(ThreadFunc func_, const string&& name)
    : _started(false),
      _joined(false),
      _thread_id(0),
      _tid(0),
      _func(std::move(func_)),
      _name(name),
      _latch(1) {
  numCreated_++;
}

Thread::~Thread() {
  if (_started && !_joined) pthread_detach(_thread_id);
}

void Thread::start() {
  assert(!_started);
  _started = true;
  // TODO consider change it to shared_ptr
  auto* pfunc = new ThreadFunc(std::bind(suduo::_detail::run_thread_func, _func,
                                         &_thread_id, &_tid, &_latch, _name));
  int err = 0;
  if (err = pthread_create(&_thread_id, nullptr, &suduo::_detail::run, pfunc)) {
    _started = false;
    delete pfunc;
    // TODO log error
  } else {
    _latch.wait();
    // assert(tid_ > 0);
    // printf("thread_create success\n");
    // TODO assert it do create a thread
  }
}

void Thread::join() {
  assert(_started);
  assert(!_joined);
  _joined = true;
  int err = 0;
  if ((err = pthread_join(_thread_id, nullptr))) {
    switch (err) {
      case EDEADLK:
        printf("A deadlock was detected\n");
        break;
      case EINVAL:
        printf("thread is not a joinable thread\n");
        break;
      case ESRCH:
        printf("No thread with the ID thread could be found.\n");
        break;
      default:
        printf("unknow error\n");
        break;
    }
    // TODO handle error
  }
  // printf("join success");
}
