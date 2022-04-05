#include "ThreadPoll.h"

#include <cstddef>
#include <functional>
#include <string>

#include "suduo/base/Mutex.h"
#include "suduo/base/Thread.h"
using ThreadPoll = suduo::ThreadPoll;

ThreadPoll::ThreadPoll(const std::string& name)
    : _mutex(),
      not_full(_mutex),
      not_empty(_mutex), /*both are not sure is safe*/
      _name(name),
      thread_init_task(),
      max_queue_size(0),
      running(false) {}
ThreadPoll::~ThreadPoll() {
  if (running) {
    stop();
  }
}

void ThreadPoll::start(int threads_num) {
  running = true;
  thread_poll.reserve(threads_num);
  for (int i = 0; i < threads_num; i++) {
    // std::string thrad_name = "Thread" + std::to_string(i);
    thread_poll.emplace_back(
        new suduo::Thread(std::bind(&ThreadPoll::thread_func, this),
                          "Thread" + std::to_string(i)));
    thread_poll[i]->start();
  }
}

void ThreadPoll::stop() {
  MutexLockGuard lock(_mutex);
  running = false;
  not_full.notifyAll();
  not_empty.notifyAll();
  for (auto& i : thread_poll) i->join();
}

void ThreadPoll::run(Task& task) {
  if (thread_poll.empty()) {
    task();
  } else {
    MutexLockGuard lock(_mutex);
    while (is_full() && running) {
      not_full.wait();
    }
    if (!running) return;
    task_queue.push_back(task);
    not_empty.notify();
  }
}

size_t ThreadPoll::queue_size() const {
  suduo::MutexLockGuard lock(_mutex);
  return task_queue.size();
}

ThreadPoll::Task ThreadPoll::get_next_task() {
  MutexLockGuard lock(_mutex);
  while (task_queue.empty() && running) {
    not_empty.wait();
  }
  // if(!running) return;
  Task task;
  if (!task_queue.empty()) {
    task = task_queue.front();
    task_queue.pop_front();
    if (max_queue_size > 0) {
      not_empty.notify();
    }
  }

  return task;
}

bool ThreadPoll::is_full() const {
  return max_queue_size > 0 && queue_size() >= max_queue_size;
}

void ThreadPoll::thread_func() {
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