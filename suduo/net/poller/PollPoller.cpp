#include "suduo/net/poller/PollPoller.h"

#include <poll.h>

#include "suduo/base/Logger.h"
#include "suduo/net/Channel.h"

using PollPoller = suduo::net::PollPoller;
using namespace suduo;

PollPoller::PollPoller(EventLoop* loop) : Poller(loop) {}

PollPoller::~PollPoller() = default;

Timestamp PollPoller::poll(int timeout_ms, ChannelList* active_channels) {
  int events_num = ::poll(_poll_fds.data(), _poll_fds.size(), timeout_ms);
  int saved_errno = errno;

  Timestamp now(Timestamp::now());

  if (events_num > 0) {
    LOG_TRACE << events_num << " events happened";
    fill_active_channels(events_num, active_channels);
  } else if (events_num == 0) {
    LOG_TRACE << " nothing happened";
  } else {
    LOG_SYSERR << "EPollPoller::poll()" << saved_errno;
  }

  return now;
}

void PollPoller::fill_active_channels(int events_num,
                                      ChannelList* active_channels) const {
  for (auto pfd = _poll_fds.begin(); pfd != _poll_fds.end() && events_num > 0;
       pfd++) {
    if (pfd->revents > 0) {
      --events_num;
      auto ch = _channels.find(pfd->fd);
      if (ch != _channels.end()) {
        Channel* channel = ch->second;
        channel->set_revents(pfd->revents);
        active_channels->push_back(channel);
      }
    }
  }
}

void PollPoller::update_channel(Channel* channel) {
  Poller::assert_in_loop_thread();

  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events();

  if (channel->index() < 0) {
    assert(_channels.find(channel->fd()) == _channels.end());

    pollfd pfd;
    pfd.fd = channel->fd();
    pfd.events = channel->events();
    pfd.revents = 0;
    _poll_fds.push_back(pfd);

    int idx = _poll_fds.size() - 1;
    channel->set_index(idx);
    _channels[pfd.fd] = channel;
  } else {
    assert(_channels.find(channel->fd()) != _channels.end());
    assert(_channels[channel->fd()] == channel);

    int idx = channel->index();

    assert(0 <= idx && idx < static_cast<int>(_poll_fds.size()));

    pollfd& pfd = _poll_fds[idx];

    assert(pfd.fd == channel->fd() || pfd.fd == -channel->fd() - 1);

    pfd.fd = channel->fd();
    pfd.events = channel->events();
    pfd.revents = 0;
    if (channel->is_disable_all()) {
      pfd.fd = -channel->fd() - 1;
    }
  }
}

void PollPoller::remove_channel(Channel* channel) {
  Poller::assert_in_loop_thread();

  LOG_TRACE << "fd = " << channel->fd();

  assert(_channels.find(channel->fd()) != _channels.end());
  assert(_channels[channel->fd()] == channel);
  assert(channel->is_disable_all());

  int idx = channel->index();
  assert(0 <= idx && idx < static_cast<int>(_poll_fds.size()));

  const struct pollfd& pfd = _poll_fds[idx];

  assert(pfd.fd == -channel->fd() - 1 && pfd.events == channel->events());

  size_t n = _channels.erase(channel->fd());

  assert(n == 1);

  if (idx == _poll_fds.size() - 1) {
    _poll_fds.pop_back();
  } else {
    int channel_at_end = _poll_fds.back().fd;
    std::iter_swap(_poll_fds.begin() + idx, _poll_fds.end() - 1);
    if (channel_at_end < 0) {
      channel_at_end = -channel_at_end - 1;
    }
    _channels[channel_at_end]->set_index(idx);
    _poll_fds.pop_back();
  }
}