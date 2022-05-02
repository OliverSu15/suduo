#include "suduo/net2/Poller.h"

#include "suduo/net2/Channel.h"

using Poller = suduo::net::Poller;

Poller::Poller(EventLoop* loop) : _loop(loop) {}

Poller::~Poller() = default;

bool Poller::has_channel(Channel* channel) const {
  assert_in_loop_thread();
  auto it = _channels.find(channel->fd());
  return it != _channels.end() && it->second == channel;
}