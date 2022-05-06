#include "suduo/net/Poller.h"

#include "suduo/net/Channel.h"

using Poller = suduo::net::Poller;

Poller::Poller(EventLoop* loop) : _loop(loop) {}

Poller::~Poller() = default;

bool Poller::has_channel(Channel* channel) const {
  assert_in_loop_thread();
  auto it = _channels.find(channel->fd());
  return it != _channels.end() && it->second == channel;
}