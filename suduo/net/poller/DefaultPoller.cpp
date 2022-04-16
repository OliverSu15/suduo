#include <cstdlib>

#include "suduo/base/Logger.h"
#include "suduo/net/Poller.h"
#include "suduo/net/poller/EPollPoller.h"
#include "suduo/net/poller/PollPoller.h"

using Poller = suduo::net::Poller;

Poller* Poller::new_dafault_poller(EventLoop* loop) {
  if (::getenv("SUDUO_USE_POLL")) {
    return new PollPoller(loop);
  } else {
    LOG_INFO << "using EPollPoller";
    return new EPollPoller(loop);
  }
  // TODO
}