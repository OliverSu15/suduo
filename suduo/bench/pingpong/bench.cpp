// Benchmark inspired by libevent/test/bench.c
// See also: http://libev.schmorp.de/bench.html

#include <stdio.h>
#include <sys/resource.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <functional>

#include "suduo/base/Logger.h"
#include "suduo/base/Thread.h"
#include "suduo/net/Channel.h"
#include "suduo/net/EventLoop.h"

using namespace suduo;
using namespace suduo::net;

std::vector<int> g_pipes;
int numPipes;
int numActive;
int numWrites;
EventLoop* g_loop;
std::vector<std::unique_ptr<Channel>> g_channels;

int g_reads, g_writes, g_fired;

void readCallback(Timestamp, int fd, int idx) {
  char ch;

  g_reads += static_cast<int>(::recv(fd, &ch, sizeof(ch), 0));
  if (g_writes > 0) {
    int widx = idx + 1;
    if (widx >= numPipes) {
      widx -= numPipes;
    }
    ::send(g_pipes[2 * widx + 1], "m", 1, 0);
    g_writes--;
    g_fired++;
  }
  if (g_fired == g_reads) {
    g_loop->quit();
  }
}

std::pair<int, int> runOnce() {
  Timestamp beforeInit(Timestamp::now());
  for (int i = 0; i < numPipes; ++i) {
    Channel& channel = *g_channels[i];
    channel.set_read_callback(
        std::bind(readCallback, std::placeholders::_1, channel.fd(), i));
    channel.enable_reading();
  }

  int space = numPipes / numActive;
  space *= 2;
  for (int i = 0; i < numActive; ++i) {
    ::send(g_pipes[i * space + 1], "m", 1, 0);
  }

  g_fired = numActive;
  g_reads = 0;
  g_writes = numWrites;
  Timestamp beforeLoop(Timestamp::now());
  g_loop->loop();

  Timestamp end(Timestamp::now());

  int iterTime = (end - beforeInit).get_microseconds_in_int64();
  int loopTime = (end - beforeLoop).get_microseconds_in_int64();
  return std::make_pair(iterTime, loopTime);
}

int main(int argc, char* argv[]) {
  numPipes = 100;
  numActive = 1;
  numWrites = 100;
  int c;
  while ((c = getopt(argc, argv, "n:a:w:")) != -1) {
    switch (c) {
      case 'n':
        numPipes = atoi(optarg);
        break;
      case 'a':
        numActive = atoi(optarg);
        break;
      case 'w':
        numWrites = atoi(optarg);
        break;
      default:
        fprintf(stderr, "Illegal argument \"%c\"\n", c);
        return 1;
    }
  }

  struct rlimit rl;
  rl.rlim_cur = rl.rlim_max = numPipes * 2 + 50;
  if (::setrlimit(RLIMIT_NOFILE, &rl) == -1) {
    perror("setrlimit");
    // return 1;  // comment out this line if under valgrind
  }
  g_pipes.resize(2 * numPipes);
  for (int i = 0; i < numPipes; ++i) {
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, &g_pipes[i * 2]) == -1) {
      perror("pipe");
      return 1;
    }
  }

  EventLoop loop;
  g_loop = &loop;

  for (int i = 0; i < numPipes; ++i) {
    Channel* channel = new Channel(loop.poller(), g_pipes[i * 2]);
    g_channels.emplace_back(channel);
  }
  double first_count = 0;
  double second_count = 0;
  for (int i = 0; i < 25; ++i) {
    printf("%d times", i);
    std::pair<int, int> t = runOnce();
    if (i != 0) {
      first_count += t.first;
      second_count += t.second;
    }
    printf("%8d %8d\n", t.first, t.second);
  }
  printf("AVG:%f %f\n", first_count / 24, second_count / 24);

  for (const auto& channel : g_channels) {
    channel->disable_all();
    channel->remove();
  }
  g_channels.clear();
}
