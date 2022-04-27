#include "suduo/net/poller/EPollPoller.h"

#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include "suduo/base/Logger.h"
#include "suduo/net/Channel.h"
using namespace suduo::net;
using namespace suduo;

using EPollPoller = suduo::net::EPollPoller;

static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

inline void memZero(void* p, size_t n) { memset(p, 0, n); }

enum Action { New = -1, Added = 1, Deleted = 2 };

EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      _epoll_fd(epoll_create1(EPOLL_CLOEXEC)),
      _events(init_event_list_size) {
  if (_epoll_fd < 0) {
    LOG_SYSFATAL << "EPollPoller::EPollPoller(): failed at epoll_create1";
  }
}

EPollPoller::~EPollPoller() { close(_epoll_fd); }

Timestamp EPollPoller::poll(int timeout_ms, ChannelList* active_channels) {
  LOG_TRACE << "fd total count " << _channels.size();

  int event_nums =
      epoll_wait(_epoll_fd, _events.data(), _events.size(), timeout_ms);

  int saved_errno = errno;
  Timestamp now(Timestamp::now());

  if (event_nums > 0) {
    LOG_TRACE << event_nums << " events happened";

    fill_active_channels(event_nums, active_channels);
    if (event_nums == _events.size()) {
      _events.reserve(_events.size() * 2);
    }

  } else if (event_nums == 0) {
    LOG_TRACE << "nothing happened";
  } else {
    if (saved_errno != EINTR) {
      // errno = saved_errno;
      LOG_SYSERR << "EPollPoller::poll()" << saved_errno;
    }
  }

  return now;
}

void EPollPoller::fill_active_channels(int events_num,
                                       ChannelList* active_channels) const {
  assert(events_num <= _events.size());

  for (int i = 0; i < events_num; i++) {
    Channel* channel = static_cast<Channel*>(_events[i].data.ptr);
    channel->set_revents(_events[i].events);
    active_channels->push_back(channel);
  }
}

void EPollPoller::update_channel(Channel* channel) {
  Poller::assert_in_loop_thread();

  const int index = channel->index();

  LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events()
            << " index = " << index;

  if (index == Action::New || index == Action::Deleted) {
    int fd = channel->fd();

    if (index == Action::New) {
      assert(_channels.find(fd) == _channels.end());
      _channels[fd] = channel;
    } else {
      assert(_channels.find(fd) != _channels.end());
      assert(_channels[fd] == channel);
    }

    channel->set_index(Action::Added);
    update(EPOLL_CTL_ADD, channel);
  } else {
    int fd = channel->fd();

    assert(_channels.find(fd) != _channels.end());
    assert(_channels[fd] == channel);
    assert(index == Action::Added);

    if (channel->is_none_event()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_index(Action::Deleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void EPollPoller::remove_channel(Channel* channel) {
  Poller::assert_in_loop_thread();

  int fd = channel->fd();

  LOG_TRACE << "fd = " << fd;

  assert(_channels.find(fd) != _channels.end());
  assert(_channels[fd] == channel);
  assert(channel->is_none_event());

  int index = channel->index();
  assert(index == Action::Added || index == Action::Deleted);
  size_t n = _channels.erase(fd);

  assert(n == 1);

  if (index == Action::Added) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_index(Action::New);
}

void EPollPoller::update(int operation, Channel* channel) {
  epoll_event event;
  memZero(&event, sizeof event);
  event.events = channel->events();
  event.data.ptr = channel;
  int fd = channel->fd();

  LOG_TRACE << "epoll_ctl op = " << operation_to_string(operation)
            << " fd = " << fd << " event = { " << channel->events_to_string()
            << " }";

  ERROR_CHECK(epoll_ctl(_epoll_fd, operation, fd, &event));
}

const char* EPollPoller::operation_to_string(int op) {
  switch (op) {
    case EPOLL_CTL_ADD:
      return "ADD";
    case EPOLL_CTL_DEL:
      return "DEL";
    case EPOLL_CTL_MOD:
      return "MOD";
    default:
      assert(false && "ERROR op");
      return "Unknown Operation";
  }
}