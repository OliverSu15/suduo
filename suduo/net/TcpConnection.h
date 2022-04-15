#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H
#include <any>
#include <memory>

#include "Callbacks.h"
#include "suduo/base/noncopyable.h"
#include "suduo/net/Buffer.h"
#include "suduo/net/InetAddress.h"
struct tcp_info;

namespace suduo {
namespace net {
class Channel;
class EventLoop;
class Socket;
class TcpConnection : noncopyable,
                      public std::enable_shared_from_this<TcpConnection> {
 public:
  TcpConnection(EventLoop* loop, const std::string& name, int sock_fd,
                const InetAddress& local_addr, const InetAddress& peer_addr);
  ~TcpConnection();

  EventLoop* get_loop() const { return _loop; }
  const std::string& name() const { return _name; }
  const InetAddress& local_address() const { return _local_addr; }
  const InetAddress& peer_address() const { return _peer_addr; }
  bool connected() const { return _state == Connected; }
  bool disconnected() const { return _state == Disconnected; }

  bool get_tcp_info(tcp_info* info) const;
  std::string get_tcp_info_string() const;

  void send(const void* message, int len);
  void send(const std::string& message);
  void send(Buffer* message);
  void shutdown();

  void force_close();
  void force_close_with_delay(double seconds);
  void set_tcp_no_delay(bool on);

  void start_read();
  void stop_read();
  bool is_reading() const { return _reading; }

  void set_context(const std::any& context) { _context = context; }
  const std::any& get_context() const { return _context; }
  std::any* get_mutable_context() { return &_context; }

  void set_connection_callback(const ConnectionCallback& cb) {
    _connection_callback = cb;
  }
  void set_message_callback(const MessageCallback& cb) {
    _message_callback = cb;
  }
  void set_write_complete_callback(const WriteCompleteCallback& cb) {
    _write_complete_callback = cb;
  }
  void set_high_water_mark_callback(const HighWaterMarkCallback& cb,
                                    size_t high_water_mark) {
    _high_water_mark_callback = cb;
    _hight_water_mark = high_water_mark;
  }

  Buffer* input_buffer() { return &_input_buffer; }
  Buffer* output_buffer() { return &_output_buffer; }

  void set_close_callback(const CloseCallback& cb) { _close_callback = cb; }

  void connect_established();
  void connect_destroyed();

 private:
  enum StateE { Disconnected, Connecting, Connected, Disconnecting };

  void handle_read(Timestamp receive_time);
  void handle_write();
  void handle_close();
  void handle_error();

  void send_in_loop(const std::string& message);
  void send_in_loop(const void* message, size_t len);
  void shutdown_in_loop();

  void force_close_in_loop();
  void set_state(StateE s) { _state = s; }
  const char* stateToString() const;
  void start_read_in_loop();
  void stop_read_in_loop();

  EventLoop* _loop;
  const std::string _name;
  StateE _state;  // FIXME: use atomic variable
  bool _reading;
  // we don't expose those classes to client.
  std::unique_ptr<Socket> _socket;
  std::unique_ptr<Channel> _channel;
  const InetAddress _local_addr;
  const InetAddress _peer_addr;
  ConnectionCallback _connection_callback;
  MessageCallback _message_callback;
  WriteCompleteCallback _write_complete_callback;
  HighWaterMarkCallback _high_water_mark_callback;
  CloseCallback _close_callback;
  size_t _hight_water_mark;
  Buffer _input_buffer;
  Buffer _output_buffer;
  std::any _context;
};
// typedef Tcp_connection_ptr
using Tcp_connection_ptr = std::shared_ptr<TcpConnection>;
}  // namespace net
}  // namespace suduo
#endif