#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#include <functional>

#include "suduo/base/noncopyable.h"
#include "suduo/net/Channel.h"
#include "suduo/net/Socket.h"
namespace suduo {
namespace net {
class EventLoop;
class InetAddress;
class Acceptor : noncopyable {
 public:
  using NewConnectionCallback =
      std::function<void(int sock_fd, const InetAddress&)>;

  Acceptor(EventLoop* loop, const InetAddress& listen_addr, bool reuse_port);
  ~Acceptor();

  void set_new_connection_callback(const NewConnectionCallback& cb) {
    _callback = std::move(cb);
  }

  void listen();

  bool listening() const { return _listening; }

 private:
  void handle_read();

  EventLoop* _loop;
  Socket _accept_socket;
  Channel _accept_Channel;
  NewConnectionCallback _callback;
  bool _listening;
  int _idle_fd;
};
}  // namespace net
}  // namespace suduo
#endif