#include "Thread.h"

#include <asm-generic/errno-base.h>
#include <asm-generic/errno.h>
#include <pthread.h>

#include <cassert>
#include <cstdio>
#include <ctime>
#include <exception>
#include <functional>
#include <memory>

#include "suduo/base/CurrentThreadInfo.h"
namespace suduo {
namespace _detail {
using Func = std::function<void()>;
void run_thread_func(Func& _func, pthread_t* _thread_id, pid_t* tid,
                     std::string& _name) {
  *tid = Current_thread_info::tid();
  tid = nullptr;
  // printf("%u run to here\n", *_thread_id);
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
      _name(name) {
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
                                         &_thread_id, &_tid, _name));
  int err = 0;
  if (err = pthread_create(&_thread_id, nullptr, &suduo::_detail::run, pfunc)) {
    _started = false;
    delete pfunc;
    // TODO log error
  } else {
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
  if (err = pthread_join(_thread_id, nullptr)) {
    switch (err) {
      case EDEADLK:
        printf("A deadlock was detected");
        break;
      case EINVAL:
        printf("thread is not a joinable thread");
        break;
      case ESRCH:
        printf("No thread with the ID thread could be found.");
        break;
      default:
        printf("unknow error");
        break;
    }
    // TODO handle error
  }
  // printf("join success");
}
