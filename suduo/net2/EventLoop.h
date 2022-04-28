#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H
#include <atomic>

#include "suduo/base/noncopyable.h"
#include "suduo/net/EventLoop.h"
namespace suduo {
namespace net {
class EventLoop : noncopyable {
 public:
  EventLoop() = default;
  ~EventLoop();

  void loop();
  void stop();

 private:
  std::atomic_bool _running;
};
}  // namespace net
}  // namespace suduo

#endif