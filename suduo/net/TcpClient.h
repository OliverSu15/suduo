#ifndef TCP_CLIENT
#define TCP_CLIENT
#include <memory>
#include <string>
#include <utility>

#include "suduo/base/Mutex.h"
#include "suduo/base/noncopyable.h"
#include "suduo/net/Callbacks.h"
#include "suduo/net/Channel.h"
#include "suduo/net/InetAddress.h"
namespace suduo {
namespace net {
class Connector;
using ConnectorPtr = std::shared_ptr<Connector>;
class TcpClient : noncopyable {
 public:
  TcpClient(EventLoop* loop, const InetAddress& server_addr,
            const std::string& name);
  ~TcpClient();

  void connect();
  void disconnect();
  void stop();

  TcpConnectionPtr connection() const {
    MutexLockGuard lock(_mutex);
    return _connection;
  }

  EventLoop* get_loop() const { return _loop; }
  bool retry() const { return _retry; }
  void enable_retry() { _retry = true; }
  const std::string& name() const { return _name; }

  void set_connection_callback(ConnectionCallback cb) {
    _connection_callback = std::move(cb);
  }

  void set_message_callback(MessageCallback cb) {
    _message_callback = std::move(cb);
  }

  void set_write_complete_callback(WriteCompleteCallback cb) {
    _write_complete_callback = std::move(cb);
  }

 private:
  void new_connection(int sock_fd);
  void remove_connection(const TcpConnectionPtr& conn);

  EventLoop* _loop;
  ConnectorPtr _connector;
  const std::string _name;
  ConnectionCallback _connection_callback;
  MessageCallback _message_callback;
  WriteCompleteCallback _write_complete_callback;
  bool _retry;
  bool _connect;
  int _next_conn_id;
  mutable MutexLock _mutex;
  TcpConnectionPtr _connection;
};
}  // namespace net
}  // namespace suduo
#endif