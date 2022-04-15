#ifndef TCP_SERVER_H
#define TCP_SERVER_H
#include <atomic>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <string>

#include "suduo/base/Thread.h"
#include "suduo/base/noncopyable.h"
#include "suduo/net/Acceptor.h"
#include "suduo/net/Callbacks.h"
#include "suduo/net/Channel.h"
#include "suduo/net/EventLoopThreadPool.h"
#include "suduo/net/InetAddress.h"
#include "suduo/net/TcpClient.h"
namespace suduo {
namespace net {
class Acceptor;
class EventLoop;
class EventLoopThreadPool;
class TcpServer : noncopyable {
 public:
  using ThreadInitCallback = std::function<void(EventLoop*)>;
  enum Option { NoReusePort, ReusePort };

  TcpServer(EventLoop* loop, const InetAddress& listen_addr, const string& name,
            Option option = NoReusePort);
  ~TcpServer();

  const string& ip_port() const { return _ip_port; }
  const string& name() const { return _name; }
  EventLoop* get_loop() const { return _loop; }

  void set_thread_num(int threads_num);
  void set_thread_init_callback(const ThreadInitCallback& cb) {
    _thread_init_callback = cb;
  }
  std::shared_ptr<EventLoopThreadPool> thread_pool() { return _thread_pool; }

  void start();

  void set_connection_callback(const ConnectionCallback& cb) {
    _connection_callback = cb;
  }
  void set_message_callback(const MessageCallback& cb) {
    _message_callback = cb;
  }

  void set_write_complete_callback(const WriteCompleteCallback& cb) {
    _write_complete_callback = cb;
  }

 private:
  void new_connection(int sock_fd, const InetAddress& peer_addr);
  void remove_connection(const TcpConnection& conn);
  void remove_connection_in_loop(const ConnectorPtr& conn);

  using ConnectionMap = std::map<std::string, TcpConnectionPtr>;

  EventLoop* _loop;
  const std::string _ip_port;
  const std::string _name;
  std::unique_ptr<Acceptor> _acceptor;
  std::shared_ptr<EventLoopThreadPool> _thread_pool;
  ConnectionCallback _connection_callback;
  MessageCallback _message_callback;
  WriteCompleteCallback _write_complete_callback;
  ThreadInitCallback _thread_init_callback;
  std::atomic_int32_t _started;
  int _next_conn_id;
  ConnectionMap _connections;
};
}  // namespace net
}  // namespace suduo
#endif