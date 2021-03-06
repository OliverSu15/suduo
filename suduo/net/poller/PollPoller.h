#ifndef POLL_POLLER_H
#define POLL_POLLER_H
#include <netinet/in.h>
#include <pthread.h>

#include "suduo/net/Poller.h"
struct pollfd;
namespace suduo {
namespace net {
class PollPoller : public Poller {
 public:
  PollPoller(EventLoop* loop);
  ~PollPoller() override;

  Timestamp poll(int timeout_ms, ChannelList* active_channels) override;

  void update_channel(Channel* channel) override;
  void remove_channel(Channel* channel) override;

 private:
  using PollFdList = std::vector<pollfd>;  // a change

  void fill_active_channels(int events_num, ChannelList* active_channels) const;

  PollFdList _poll_fds;
};
}  // namespace net
}  // namespace suduo

#endif