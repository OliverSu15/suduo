#include "ThreadPool.h"

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
      not_full(_mutex),
      not_empty(_mutex), /*both are not sure is safe*/
      _name(name),
      max_queue_size(0),
      running(false) {}
ThreadPool::~ThreadPool() {
  if (running) {
    stop();
  }
}

void ThreadPool::start(int threads_num) {
  running = true;
  thread_pool.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    char id[32];
    snprintf(id, sizeof id, "%d", i + 1);
    // std::string thrad_name = "Thread" + std::to_string(i);
    thread_pool.emplace_back(new suduo::Thread(
        std::bind(&ThreadPool::thread_func, this), _name + id));
    thread_pool[i]->start();
  }
}

void ThreadPool::stop() {
  {
    MutexLockGuard lock(_mutex);
    running = false;
    not_empty.notifyAll();
    not_full.notifyAll();
  }
  for (auto& i : thread_pool) {
    i->join();
  }
}

void ThreadPool::run(Task task) {
  if (thread_pool.empty()) {
    task();
  } else {
    MutexLockGuard lock(_mutex);
    while (is_full() && running) {
      not_full.wait();
    }

    if (!running) return;
    task_queue.push_back(std::move(task));
    not_empty.notify();
  }
}

size_t ThreadPool::queue_size() const {
  suduo::MutexLockGuard lock(_mutex);
  return task_queue.size();
}

ThreadPool::Task ThreadPool::get_next_task() {
  MutexLockGuard lock(_mutex);
  while (task_queue.empty() && running) {
    not_empty.wait();
  }
  // if (!running) return;
  Task task;
  if (!task_queue.empty()) {
    task = task_queue.front();
    task_queue.pop_front();
    if (max_queue_size > 0) {
      not_full.notify();
    }
  }

  return task;
}

bool ThreadPool::is_full() const {
  assert(_mutex.isLockedByThisThread());
  return max_queue_size > 0 && task_queue.size() >= max_queue_size;
}

void ThreadPool::thread_func() {
  try {
    if (thread_init_task) thread_init_task();
    while (running) {
      Task task(get_next_task());
      if (task) {
        task();
      }
    }

  } catch (const std::exception& e) {
    // TODO LOG exception info
  } catch (...) {
    // TODO LOG exception info
    throw;
  }
}