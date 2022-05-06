#include <stdio.h>
#include <unistd.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

#include "suduo/base/BlockingQueue.h"
#include "suduo/base/CountDownLatch.h"
#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Logger.h"
#include "suduo/base/Thread.h"
#include "suduo/base/Timestamp.h"

bool g_verbose = false;

// suduo::Timestamp invalid() {
//   return suduo::Timestamp(suduo::Timestamp::Nanoseconds::zero());
// }

// bool valid(const suduo::Timestamp& time) {
//   return time.get_Time_Point().time_since_epoch() !=
//          suduo::Timestamp::Nanoseconds::zero();
// }
// Many threads, one queue.
class Bench {
 public:
  Bench(int numThreads) : latch_(numThreads) {
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(new suduo::Thread(
          std::bind(&Bench::threadFunc, this), suduo::string(name)));
    }
    for (auto& thr : threads_) {
      thr->start();
    }
  }

  void run(int times) {
    printf("waiting for count down latch\n");
    latch_.wait();
    LOG_INFO << threads_.size() << " threads started";
    int64_t total_delay = 0;
    for (int i = 0; i < times; ++i) {
      suduo::Timestamp now(suduo::Timestamp::now());
      queue_.push(now);
      total_delay += delay_queue_.pop();
    }
    printf("Average delay: %.3fus\n", static_cast<double>(total_delay) / times);
  }

  void joinAll() {
    for (size_t i = 0; i < threads_.size(); ++i) {
      queue_.push(suduo::Timestamp::get_invalid());
    }

    for (auto& thr : threads_) {
      thr->join();
    }
    LOG_INFO << threads_.size() << " threads stopped";
  }

 private:
  void threadFunc() {
    if (g_verbose) {
      printf("tid=%d, %s started\n", suduo::Current_thread_info::tid(),
             suduo::Current_thread_info::name());
    }

    std::map<int, int> delays;
    latch_.count_down();
    bool running = true;
    while (running) {
      suduo::Timestamp t(queue_.pop());
      suduo::Timestamp now(suduo::Timestamp::now());
      if (t.valid()) {
        int delay = static_cast<int>((now - t).get_microseconds_in_double());
        // printf("tid=%d, latency = %d us\n",
        //        suduo::CurrentThread::tid(), delay);
        ++delays[delay];
        delay_queue_.push(delay);
      }
      running = t.valid();
    }

    if (g_verbose) {
      printf("tid=%d, %s stopped\n", suduo::Current_thread_info::tid(),
             suduo::Current_thread_info::name());
      for (const auto& delay : delays) {
        printf("tid = %d, delay = %d, count = %d\n",
               suduo::Current_thread_info::tid(), delay.first, delay.second);
      }
    }
  }

  suduo::BlockingQueue<suduo::Timestamp> queue_;
  suduo::BlockingQueue<int> delay_queue_;
  suduo::CountDownLatch latch_;
  std::vector<std::unique_ptr<suduo::Thread>> threads_;
};

int main(int argc, char* argv[]) {
  int threads = argc > 1 ? atoi(argv[1]) : 1;

  Bench t(threads);
  t.run(100000);
  t.joinAll();
}
