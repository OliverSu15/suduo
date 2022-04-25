#include "ThreadPool.h"

#include <array>
#include <charconv>
#include <cstddef>
#include <cstdio>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>

#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Thread.h"
using ThreadPool = suduo::ThreadPool;

ThreadPool::ThreadPool(const std::string& name)
    : _mutex(),
      _not_full(_mutex),
      _not_empty(_mutex),
      _name(name),
      _max_queue_size(0),
      _running(false) {}

ThreadPool::~ThreadPool() {
  if (_running) {
    stop();
  }
}

void ThreadPool::start(int threads_num) {
  _running = true;
  _thread_pool.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    std::array<char, 32> id;
    auto [ptr, ec] = std::to_chars(id.begin(), id.end(), i + 1);
    // snprintf(id, sizeof id, "%d", i + 1);
    _thread_pool.emplace_back(new suduo::Thread(
        std::bind(&ThreadPool::thread_func, this), _name + id.data()));
    _thread_pool[i]->start();
  }
}

void ThreadPool::stop() {
  {
    MutexLockGuard lock(_mutex);
    _running = false;
    _not_empty.notifyAll();
    _not_full.notifyAll();
  }
  for (auto& i : _thread_pool) {
    i->join();
  }
}

void ThreadPool::run(Task task) {
  if (_thread_pool.empty()) {
    task();
  } else {
    MutexLockGuard lock(_mutex);
    while (is_full() && _running) {
      _not_full.wait();
    }

    if (!_running) return;
    _task_queue.push_back(std::move(task));
    _not_empty.notify();
  }
}

size_t ThreadPool::queue_size() const {
  suduo::MutexLockGuard lock(_mutex);
  return _task_queue.size();
}

ThreadPool::Task ThreadPool::get_next_task() {
  MutexLockGuard lock(_mutex);
  while (_task_queue.empty() && _running) {
    _not_empty.wait();
  }
  // if (!running) return;
  Task task;
  if (!_task_queue.empty()) {
    task = _task_queue.front();
    _task_queue.pop_front();
    if (_max_queue_size > 0) {
      _not_full.notify();
    }
  }

  return task;
}

bool ThreadPool::is_full() const {
  _mutex.assert_locked();
  return _max_queue_size > 0 && _task_queue.size() >= _max_queue_size;
}

void ThreadPool::thread_func() {
  // try {
  if (_thread_init_task) _thread_init_task();
  while (_running) {
    Task task(get_next_task());
    if (task) {
      task();
    }
  }

  // } catch (const std::exception& e) {
  //   // TODO LOG exception info
  // } catch (...) {
  //   // TODO LOG exception info
  //   throw;
  // }
}