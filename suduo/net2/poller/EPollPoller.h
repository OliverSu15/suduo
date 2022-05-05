#ifndef EPOLLPOLLER_H
#define EPOLLPOLLER_H
#include <vector>

#include "suduo/net/Poller.h"
struct epoll_event;

namespace suduo {

namespace net {
class EPollPoller : public Poller {
 public:
  EPollPoller(EventLoop* loop);
  ~EPollPoller() override;

  Timestamp poll(int timeout_ms, ChannelList* active_channels) override;

  void update_channel(Channel* channel) override;
  void remove_channel(Channel* channel) override;

 private:
  using EventList = std::vector<epoll_event>;

  static const int init_event_list_size = 16;
  static const char* operation_to_string(int op);

  void fill_active_channels(int event_num, ChannelList* active_channels) const;

  void update(int operation, Channel* channel);

  const int _epoll_fd;
  EventList _events;
};
}  // namespace net
}  // namespace suduo
#endif