#include <stdio.h>
#include <unistd.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "suduo/base/BlockingQueue.h"
#include "suduo/base/CountDownLatch.h"
#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Thread.h"
#include "suduo/base/Timestamp.h"

// hot potato benchmarking https://en.wikipedia.org/wiki/Hot_potato
// N threads, one hot potato.
class Bench {
 public:
  Bench(int numThreads) : startLatch_(numThreads), stopLatch_(1) {
    queues_.reserve(numThreads);
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
      queues_.emplace_back(new suduo::BlockingQueue<int>());
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(
          new suduo::Thread([this, i] { threadFunc(i); }, suduo::string(name)));
    }
  }

  void Start() {
    suduo::Timestamp start = suduo::Timestamp::now();
    for (auto& thr : threads_) {
      thr->start();
    }
    startLatch_.wait();
    suduo::Timestamp started = suduo::Timestamp::now();
    printf("all %zd threads started, %.3fms\n", threads_.size(),
           1e3 * ((started - start).get_seconds_in_double()));
  }

  void Run() {
    suduo::Timestamp start = suduo::Timestamp::now();
    const int rounds = 100003;
    queues_[0]->push(rounds);

    auto done = done_.pop();
    double elapsed = (done.second - start).get_seconds_in_double();
    printf("thread id=%d done, total %.3fms, %.3fus / round\n", done.first,
           1e3 * elapsed, 1e6 * elapsed / rounds);
  }

  void Stop() {
    suduo::Timestamp stop = suduo::Timestamp::now();
    for (const auto& queue : queues_) {
      queue->push(-1);
    }
    for (auto& thr : threads_) {
      thr->join();
    }

    suduo::Timestamp t2 = suduo::Timestamp::now();
    printf("all %zd threads joined, %.3fms\n", threads_.size(),
           1e3 * ((t2 - stop).get_seconds_in_double()));
  }

 private:
  void threadFunc(int id) {
    startLatch_.count_down();

    suduo::BlockingQueue<int>* input = queues_[id].get();
    suduo::BlockingQueue<int>* output =
        queues_[(id + 1) % queues_.size()].get();
    while (true) {
      int value = input->pop();
      if (value > 0) {
        output->push(value - 1);
        if (verbose_) {
          // printf("thread %d, got %d\n", id, value);
        }
        continue;
      }

      if (value == 0) {
        done_.push(std::make_pair(id, suduo::Timestamp::now()));
      }
      break;
    }
  }

  using TimestampQueue = suduo::BlockingQueue<std::pair<int, suduo::Timestamp>>;
  TimestampQueue done_;
  suduo::CountDownLatch startLatch_, stopLatch_;
  std::vector<std::unique_ptr<suduo::BlockingQueue<int>>> queues_;
  std::vector<std::unique_ptr<suduo::Thread>> threads_;
  const bool verbose_ = true;
};

int main(int argc, char* argv[]) {
  int threads = argc > 1 ? atoi(argv[1]) : 1;

  printf("sizeof BlockingQueue = %zd\n", sizeof(suduo::BlockingQueue<int>));
  printf("sizeof deque<int> = %zd\n", sizeof(std::deque<int>));
  Bench t(threads);
  t.Start();
  t.Run();
  t.Stop();
  // exit(0);
}
