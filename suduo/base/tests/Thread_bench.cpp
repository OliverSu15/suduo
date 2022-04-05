#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>

#include <atomic>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "suduo/base/BlockingQueue.h"
#include "suduo/base/CountDownLatch.h"
#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Mutex.h"
#include "suduo/base/Thread.h"
#include "suduo/base/Timestamp.h"

bool g_verbose = false;
suduo::MutexLock g_mutex;
std::atomic_int32_t g_count;
std::map<int, int> g_delays;

void threadFunc() {
  // printf("tid=%d\n", suduo::CurrentThread::tid());
  g_count++;
}

void threadFunc2(suduo::Timestamp start) {
  suduo::Timestamp now(suduo::Timestamp::now());
  int delay = static_cast<int>(
      (timeDifference(now, start) / (suduo::SECOND_TO_NANOSECOND)) * 1000000);
  suduo::MutexLockGuard lock(g_mutex);
  ++g_delays[delay];
}

void forkBench() {
  sleep(10);
  suduo::Timestamp start(suduo::Timestamp::now());
  int kProcesses = 10 * 1000;

  printf("Creating %d processes in serial\n", kProcesses);
  for (int i = 0; i < kProcesses; ++i) {
    pid_t child = fork();
    if (child == 0) {
      exit(0);
    } else {
      waitpid(child, NULL, 0);
    }
  }

  double timeUsed = timeDifference(suduo::Timestamp::now(), start) /
                    (suduo::SECOND_TO_NANOSECOND);
  printf("time elapsed %.3f seconds, process creation time used %.3f us\n",
         timeUsed, timeUsed * 1e6 / kProcesses);
  printf("number of created processes %d\n", kProcesses);
}

class Bench {
 public:
  Bench(int numThreads) : startLatch_(numThreads), stopLatch_(1) {
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(
          new suduo::Thread([this] { threadFunc(); }, suduo::string(name)));
    }
  }

  void Start() {
    const int numThreads = static_cast<int>(threads_.size());
    printf("Creating %d threads in parallel\n", numThreads);
    suduo::Timestamp start = suduo::Timestamp::now();

    for (auto& thr : threads_) {
      thr->start();
    }
    startLatch_.wait();
    double timeUsed = timeDifference(suduo::Timestamp::now(), start) /
                      (suduo::SECOND_TO_NANOSECOND);
    printf("all %d threads started, %.3fms total, %.3fus per thread\n",
           numThreads, 1e3 * timeUsed, 1e6 * timeUsed / numThreads);

    TimestampQueue::queue_type queue = start_.drain();
    if (g_verbose) {
      // for (const auto& [tid, ts] : queue)
      for (const auto& e : queue) {
        printf(
            "thread %d, %.0f us\n", e.first,
            (timeDifference(e.second, start) / (suduo::SECOND_TO_NANOSECOND)) *
                1e6);
      }
    }
  }

  void Stop() {
    suduo::Timestamp stop = suduo::Timestamp::now();
    stopLatch_.count_down();
    for (auto& thr : threads_) {
      thr->join();
    }

    suduo::Timestamp t2 = suduo::Timestamp::now();
    printf("all %zd threads joined, %.3fms\n", threads_.size(),
           1e3 * (timeDifference(t2, stop) / (suduo::SECOND_TO_NANOSECOND)));
    TimestampQueue::queue_type queue = done_.drain();
    if (g_verbose) {
      // for (const auto& [tid, ts] : queue)
      for (const auto& e : queue) {
        printf("thread %d, %.0f us\n", e.first,
               (timeDifference(e.second, stop) / suduo::SECOND_TO_NANOSECOND) *
                   1e6);
      }
    }
  }

 private:
  void threadFunc() {
    const int tid = suduo::Current_thread_info::tid();
    start_.push(std::make_pair(tid, suduo::Timestamp::now()));
    startLatch_.count_down();
    stopLatch_.wait();
    done_.push(std::make_pair(tid, suduo::Timestamp::now()));
  }

  using TimestampQueue = suduo::BlockingQueue<std::pair<int, suduo::Timestamp>>;
  TimestampQueue start_, run_, done_;
  suduo::CountDownLatch startLatch_, stopLatch_;
  std::vector<std::unique_ptr<suduo::Thread>> threads_;
};

int main(int argc, char* argv[]) {
  g_verbose = argc > 1;
  printf("pid=%d, tid=%d, verbose=%d\n", ::getpid(),
         suduo::Current_thread_info::tid(), g_verbose);
  suduo::Timestamp start(suduo::Timestamp::now());

  int kThreads = 100 * 1000;
  printf("Creating %d threads in serial\n", kThreads);
  for (int i = 0; i < kThreads; ++i) {
    suduo::Thread t1(threadFunc);
    t1.start();
    t1.join();
  }

  double timeUsed = timeDifference(suduo::Timestamp::now(), start) /
                    (suduo::SECOND_TO_NANOSECOND);
  printf("elapsed %.3f seconds, thread creation time %.3f us\n", timeUsed,
         timeUsed * 1e6 / kThreads);
  printf("number of created threads %d, g_count = %d\n",
         suduo::Thread::numCreated(), g_count.load());

  for (int i = 0; i < kThreads; ++i) {
    suduo::Timestamp now(suduo::Timestamp::now());
    suduo::Thread t2(std::bind(threadFunc2, now));
    t2.start();
    t2.join();
  }

  if (g_verbose) {
    suduo::MutexLockGuard lock(g_mutex);
    for (const auto& delay : g_delays) {
      printf("delay = %d, count = %d\n", delay.first, delay.second);
    }
  }

  Bench t(10000);
  t.Start();
  t.Stop();

  forkBench();
}
