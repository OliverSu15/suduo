#include "suduo/base/ThreadPool.h"

#include <stdio.h>
#include <unistd.h>  // usleep

#include <iostream>

#include "suduo/base/CountDownLatch.h"
#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Logger.h"
void print() { printf("tid=%d\n", suduo::Current_thread_info::tid()); }

void printString(const std::string& str) {
  LOG_INFO << str;
  usleep(100 * 1000);
}

void test(int maxSize) {
  LOG_WARN << "Test ThreadPool with max queue size = " << maxSize;
  suduo::ThreadPool pool("MainThreadPool");
  pool.set_max_queue_size(maxSize);
  pool.start(5);

  LOG_WARN << "Adding";
  pool.run(print);
  pool.run(print);
  for (int i = 0; i < 100; ++i) {
    char buf[32];
    snprintf(buf, sizeof buf, "task %d", i);
    pool.run(std::bind(printString, std::string(buf)));
  }
  LOG_WARN << "Done";

  suduo::CountDownLatch latch(1);
  pool.run(std::bind(&suduo::CountDownLatch::count_down, &latch));
  latch.wait();
  pool.stop();
}

/*
 * Wish we could do this in the future.
void testMove()
{
  suduo::ThreadPool pool;
  pool.start(2);

  std::unique_ptr<int> x(new int(42));
  pool.run([y = std::move(x)]{ printf("%d: %d\n", suduo::CurrentThread::tid(),
*y); }); pool.stop();
}
*/

void longTask(int num) {
  LOG_INFO << "longTask " << num;
  suduo::Current_thread_info::sleep_us(3000000);
}

void test2() {
  LOG_WARN << "Test ThreadPool by stoping early.";
  suduo::ThreadPool pool("ThreadPool");
  pool.set_max_queue_size(5);
  pool.start(3);

  suduo::Thread thread1(
      [&pool]() {
        for (int i = 0; i < 20; ++i) {
          pool.run(std::bind(longTask, i));
        }
      },
      "thread1");
  thread1.start();

  suduo::Current_thread_info::sleep_us(5000000);
  LOG_WARN << "stop pool";
  pool.stop();  // early stop

  thread1.join();
  // run() after stop()
  pool.run(print);
  LOG_WARN << "test2 Done";
}

int main() {
  test(0);
  test(1);
  test(5);
  test(10);
  test(50);
  test2();
}
