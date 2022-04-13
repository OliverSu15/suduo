#include "EventLoopThreadPool.h"

#include <cstdio>
#include <memory>

#include "suduo/net/Channel.h"
#include "suduo/net/EventLoopThread.h"

using EventLoopThreadPool = suduo::net::EventLoopThreadPool;
using namespace suduo::net;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* base_loop,
                                         const std::string& name)
    : _base_loop(base_loop),
      _name(name),
      _started(false),
      _thread_nums(0),
      _next(0) {}

EventLoopThreadPool::~EventLoopThreadPool() {}

void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
  assert(!_started);
  _base_loop->assert_in_loop_thread();
  _started = true;

  for (int i = 0; i < _thread_nums; i++) {
    char buf[_name.size() + 32];
    snprintf(buf, sizeof(buf), "%s%d", _name.c_str(), i);
    EventLoopThread* t = new EventLoopThread(cb, buf);
    _threads.push_back(std::unique_ptr<EventLoopThread>(t));
    _loops.push_back(t->start_loop());
  }
  if (_thread_nums == 0 && cb) {
    cb(_base_loop);
  }
}

EventLoop* EventLoopThreadPool::get_next_loop() {
  _base_loop->assert_in_loop_thread();
  assert(_started);
  EventLoop* loop = _base_loop;

  if (!_loops.empty()) {
    loop = _loops[_next];
    ++_next;
    if (_next >= _loops.size()) _next = 0;
  }
  return loop;
}

EventLoop* EventLoopThreadPool::get_loop_for_hash(size_t hash_code) {
  _base_loop->assert_in_loop_thread();
  EventLoop* loop = _base_loop;
  if (!_loops.empty()) {
    loop = _loops[hash_code % _loops.size()];
  }
  return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::get_all_loops() {
  _base_loop->assert_in_loop_thread();
  assert(_started);
  if (_loops.empty()) {
    return std::vector<EventLoop*>(1, _base_loop);
  } else {
    return _loops;
  }
}