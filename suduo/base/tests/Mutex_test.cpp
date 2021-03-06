#include <stdio.h>

#include <vector>

//#include "suduo/base/CountDownLatch.h"
#include <memory>
#include <vector>

#include "suduo/base/Mutex.h"
#include "suduo/base/Thread.h"
#include "suduo/base/Timestamp.h"

using namespace suduo;
using namespace std;

MutexLock g_mutex;
vector<int> g_vec;
const int kCount = 10 * 1000 * 1000;

void threadFunc() {
  // printf("threadFunc start\n");
  for (int i = 0; i < kCount; ++i) {
    MutexLockGuard lock(g_mutex);
    g_vec.push_back(i);
  }
  // printf("threadFunc stop\n");
}

int foo() __attribute__((noinline));

int g_count = 0;
int foo() {
  MutexLockGuard lock(g_mutex);
  if (!g_mutex.is_locked_by_this_thread()) {
    printf("FAIL\n");
    return -1;
  }

  ++g_count;
  return 0;
}

int main() {
  printf("sizeof pthread_mutex_t: %zd\n", sizeof(pthread_mutex_t));
  printf("sizeof Mutex: %zd\n", sizeof(MutexLock));
  printf("sizeof pthread_cond_t: %zd\n", sizeof(pthread_cond_t));
  printf("sizeof Condition: %zd\n", sizeof(Condition));
  ERROR_CHECK(foo());
  if (g_count != 1) {
    printf("MCHECK calls twice.\n");
    abort();
  }

  const int kMaxThreads = 8;
  g_vec.reserve(kMaxThreads * kCount);

  Timestamp start(Timestamp::now());
  for (int i = 0; i < kCount; ++i) {
    g_vec.push_back(i);
  }

  printf("single thread without lock %f\n",
         (Timestamp::now() - start).get_seconds_in_double());

  start = Timestamp::now();
  threadFunc();
  printf("single thread with lock %f\n",
         (Timestamp::now() - start).get_seconds_in_double());

  for (int nthreads = 1; nthreads < kMaxThreads; ++nthreads) {
    std::vector<std::unique_ptr<Thread>> threads;
    g_vec.clear();
    start = Timestamp::now();
    for (int i = 0; i < nthreads; ++i) {
      threads.emplace_back(new Thread(&threadFunc));
      threads.back()->start();
      // printf("thread %d start\n", i);
    }
    for (int i = 0; i < nthreads; ++i) {
      threads[i]->join();
      // printf("thread %d joined\n", i);
    }
    printf("%d thread(s) with lock %f\n", nthreads,
           (Timestamp::now() - start).get_seconds_in_double());
  }
}
