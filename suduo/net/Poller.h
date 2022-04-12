#ifndef POLLER_H
#define POLLER_H
#include <map>
#include <vector>

#include "suduo/base/Timestamp.h"
#include "suduo/base/noncopyable.h"
#include "suduo/net/Channel.h"
namespace suduo {
namespace net {
class Channel;
class Poller : noncopyable {
 public:
  using ChannelList = std::vector<Channel*>;

  Poller(EventLoop* loop);
  virtual ~Poller();

  virtual Timestamp poll(int timeout_ms, ChannelList* active_channels) = 0;

  virtual void update_channel(Channel* channel) = 0;

  virtual bool has_channel(Channel* channel) const;

  static Poller* new_dafault_poller(EventLoop* loop);

  void assert_in_loop_thread() const { _owner_loop->assertInLoopThread(); }

 protected:
  using ChannelMap = std::map<int, Channel*>;
  ChannelMap _channels;

 private:
  EventLoop* _owner_loop;
};
}  // namespace net
}  // namespace suduo
#endif