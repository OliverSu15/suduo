#include "Thread.h"

#include <pthread.h>

#include <cassert>
#include <exception>
#include <functional>
namespace suduo {
namespace _detail {
using Func = std::function<void()>;
void run_thread_func(Func& _func, pthread_t _thread_id, std::string& _name) {
  pthread_setname_np(_thread_id, _name.c_str());
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

Thread::Thread(ThreadFunc func_, const string&& name)
    : _started(false),
      _joined(false),
      _thread_id(0),
      _tid(0),
      _func(std::move(func_)),
      _name(name) {}

Thread::~Thread() {
  if (_started && !_joined) pthread_detach(_thread_id);
}

void Thread::start() {
  assert(!_started);
  _started = true;
  // TODO consider change it to shared_ptr
  auto* pfunc = new ThreadFunc(
      std::bind(suduo::_detail::run_thread_func, _func, _thread_id, _name));
  int err = 0;
  if (err = pthread_create(&_thread_id, nullptr, &suduo::_detail::run, pfunc)) {
    _started = false;
    delete pfunc;
    // TODO log error
  } else {
    // TODO assert it do create a thread
  }
}

void Thread::join() {
  assert(_started);
  assert(!_joined);
  _joined = true;
  int err = 0;
  if (err = pthread_join(_thread_id, nullptr)) {
    // TODO handle error
  }
}
