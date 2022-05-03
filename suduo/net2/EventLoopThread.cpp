#include "suduo/net2/EventLoopThread.h"

#include "suduo/net2/EventLoop.h"

using namespace suduo::net;
using EventLoopThread = suduo::net::EventLoopThread;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    : _loop(nullptr),
      _thread(std::bind(&EventLoopThread::thread_func, this)),
      _mutex(),
      _cond(_mutex),
      _callback(cb) {}

EventLoopThread::~EventLoopThread() {
  if (_loop != nullptr) {
    _loop->quit();
    _thread.join();
  }
}

EventLoop* EventLoopThread::start_loop() {
  assert(!_thread.started());
  _thread.start();
  EventLoop* loop = nullptr;
  {
    MutexLockGuard lock(_mutex);
    while (_loop == nullptr) {
      _cond.wait();
    }
    loop = _loop;
  }
  return loop;
}

void EventLoopThread::thread_func() {
  EventLoop loop;
  if (_callback) {
    _callback(&loop);
  }
  {
    MutexLockGuard lock(_mutex);
    _loop = &loop;
    _cond.notify();
  }
  loop.loop();
  MutexLockGuard lock(_mutex);
  _loop = nullptr;
}
