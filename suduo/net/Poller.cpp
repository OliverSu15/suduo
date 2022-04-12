#include "suduo/net/Poller.h"

#include <string>

#include "suduo/net/Channel.h"

using Poller = suduo::net::Poller;

Poller::Poller(EventLoop* loop) : _owner_loop(loop) {}

Poller::~Poller() = default;

bool Poller::has_channel(Channel* channel) const {
  assert_in_loop_thread();
  ChannelMap::const_iterator it = _channels.find(channel->fd());
  return it != _channels.end() && it->second == channel;
}