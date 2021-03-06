#include "suduo/base/BoundedBlockingQueue.h"

#include <stdio.h>
#include <unistd.h>

#include <memory>
#include <string>
#include <vector>

#include "suduo/base/CountDownLatch.h"
#include "suduo/base/Thread.h"

class Test {
 public:
  Test(int numThreads) : queue_(5), latch_(numThreads) {
    threads_.reserve(numThreads);
    for (int i = 0; i < numThreads; ++i) {
      char name[32];
      snprintf(name, sizeof name, "work thread %d", i);
      threads_.emplace_back(new suduo::Thread(
          std::bind(&Test::threadFunc, this), suduo::string(name)));
    }
    for (auto& thr : threads_) {
      thr->start();
    }
  }

  void run(int times) {
    printf("waiting for count down latch\n");
    latch_.wait();
    printf("all threads started\n");
    for (int i = 0; i < times; ++i) {
      char buf[32];
      snprintf(buf, sizeof buf, "hello %d", i);
      queue_.push(buf);
      printf("tid=%d, push data = %s, size = %zd\n",
             suduo::Current_thread_info::tid(), buf, queue_.size());
    }
  }

  void joinAll() {
    for (size_t i = 0; i < threads_.size(); ++i) {
      queue_.push("stop");
    }

    for (auto& thr : threads_) {
      thr->join();
    }
  }

 private:
  void threadFunc() {
    printf("tid=%d, %s started\n", suduo::Current_thread_info::tid(),
           suduo::Current_thread_info::name());

    latch_.count_down();
    bool running = true;
    while (running) {
      std::string d(queue_.pop());
      printf("tid=%d, get data = %s, size = %zd\n",
             suduo::Current_thread_info::tid(), d.c_str(), queue_.size());
      running = (d != "stop");
    }

    printf("tid=%d, %s stopped\n", suduo::Current_thread_info::tid(),
           suduo::Current_thread_info::name());
  }

  suduo::BoundedBlockingQueue<std::string> queue_;
  suduo::CountDownLatch latch_;
  std::vector<std::unique_ptr<suduo::Thread>> threads_;
};

void testMove() {
#if BOOST_VERSION >= 105500L
  suduo::BoundedBlockingQueue<std::unique_ptr<int>> queue(10);
  queue.push(std::unique_ptr<int>(new int(42)));
  std::unique_ptr<int> x = queue.take();
  printf("took %d\n", *x);
  *x = 123;
  queue.push(std::move(x));
  std::unique_ptr<int> y;
  y = queue.take();
  printf("took %d\n", *y);
#endif
}

int main() {
  printf("pid=%d, tid=%d\n", ::getpid(), suduo::Current_thread_info::tid());
  testMove();
  Test t(1);
  t.run(100);
  t.joinAll();

  printf("number of created threads %d\n", suduo::Thread::numCreated());
}
