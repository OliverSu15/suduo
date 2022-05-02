#ifndef POLLER_H
#define POLLER_H
#include <map>
#include <vector>

#include "suduo/base/noncopyable.h"
#include "suduo/net2/EventLoop.h"
namespace suduo {
class Channel;
namespace net {
class Poller : noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;

  Poller(EventLoop* loop);
  virtual ~Poller();

  virtual Timestamp poll(int timeout_ms, ChannelList* active_channels) = 0;

  virtual void update_channel(Channel* channel) = 0;
  virtual void remove_channel(Channel* channel) = 0;

  virtual bool has_channel(Channel* channel) const;

  static Poller* new_default_poller(EventLoop* loop);

  void assert_in_loop_thread() const { _loop->assert_in_loop_thread(); }

 protected:
  using ChannelMap = std::map<int, Channel*>;

  ChannelMap _channels;

 private:
  EventLoop* _loop;
};
}  // namespace net
}  // namespace suduo
#endif