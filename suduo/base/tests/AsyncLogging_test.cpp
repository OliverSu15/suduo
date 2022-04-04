#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>

#include <cstdio>

#include "suduo/base/AsyncLogger.h"
#include "suduo/base/CurrentThreadInfo.h"
#include "suduo/base/Logger.h"
#include "suduo/base/Timestamp.h"

off_t kRollSize = 500 * 1000 * 1000;

suduo::AsyncLogger* g_asyncLog = NULL;

void asyncOutput(const char* msg, int len) { g_asyncLog->append(msg, len); }

void bench(bool longLog) {
  suduo::Logger::set_output_function(asyncOutput);

  int cnt = 0;
  const int kBatch = 1000;
  suduo::string empty = " ";
  suduo::string longStr(3000, 'X');
  longStr += " ";
  for (int t = 0; t < 30; ++t) {
    suduo::Timestamp start = suduo::Timestamp::now();
    for (int i = 0; i < kBatch; ++i) {
      LOG_INFO << "Hello 0123456789"
               << " abcdefghijklmnopqrstuvwxyz " << (longLog ? longStr : empty)
               << cnt;
      ++cnt;
    }
    suduo::Timestamp end = suduo::Timestamp::now();
    printf("%f\n",
           ((timeDifference(end, start) / suduo::MICROSECOND_TO_NANOSECOND) /
            kBatch));
    struct timespec ts = {0, 500 * 1000 * 1000};
    nanosleep(&ts, NULL);
  }
}

int main(int argc, char* argv[]) {
  {
    // set max virtual memory to 2GB.
    size_t kOneGB = 1000 * 1024 * 1024;
    rlimit rl = {2 * kOneGB, 2 * kOneGB};
    setrlimit(RLIMIT_AS, &rl);
  }

  printf("pid = %d\n", getpid());

  char name[256] = {'\0'};

  strncpy(name, argv[0], sizeof name - 1);
  suduo::AsyncLogger log(::basename(name), kRollSize);
  log.start();
  g_asyncLog = &log;

  bool longLog = argc > 1;
  bench(longLog);
}
