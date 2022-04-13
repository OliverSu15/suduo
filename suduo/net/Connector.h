#ifndef CONNECTOR_H
#define CONNECTOR_H
#include "suduo/base/noncopyable.h"
#include "suduo/net/Channel.h"
#include "suduo/net/InetAddress.h"
#inlcude "suduo/net/Callbacks.h"
#include <memory>
namespace suduo {
namespace net {
class Channel;
class EventLoop;
class Connector : noncopyable {
 public:
  using NewConnectionCallback = std::function<void(int sockfd)>;

  Connector(EventLoop* loop, const InetAddress& server_addr);
  ~Connector();

  void set_new_connection_callback(const NewConnectionCallback& cb) {
    _new_connection_callback = cb;
  }

  void start();
  void restart();
  void stop();

  const InetAddress& server_address() const { return _server_addr; }

 private:
  enum State { Disconnected, Connecting, Connected };
  static const int MAX_RETRY_DELAY_MS = 30 * 1000;
  static const int INIT_RETRY_DELAY_MS = 500;
  void set_state(State s) { _state = s; }
  void start_in_loop();
  void stop_in_loop();
  void connect();
  void connecting(int sockfd);
  void handle_write();
  void handle_error();
  void retry(int sockfd);
  int remove_and_reset_channel();
  void reset_channel();

  EventLoop* _loop;
  InetAddress _server_addr;
  bool _connect;
  State _state;
  std::unique_ptr<Channel> _channel;
  NewConnectionCallback _new_connection_callback;
  int retry_delay_ms;
};
}  // namespace net
}  // namespace suduo

#endif;