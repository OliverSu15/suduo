#ifndef CHANNEL_H
#define CHANNEL_H
#include <poll.h>

#include <cstdint>
#include <functional>
#include <memory>

#include "suduo/base/Timestamp.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
namespace net {
class EventLoop;
class Poller;
class Channel : noncopyable {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(Timestamp)>;

  Channel(EventLoop* loop, int fd, Poller* poller);
  ~Channel();

  void handle_event(const Timestamp& receivetime);

  void set_read_callback(ReadEventCallback cb) {
    _read_callback = std::move(cb);
  }
  void set_write_callback(EventCallback cb) { _write_callback = std::move(cb); }
  void set_close_callback(EventCallback cb) { _close_callback = std::move(cb); }
  void set_error_callback(EventCallback cb) { _error_callback = std::move(cb); }

  void tie(const std::shared_ptr<void>& obj) {
    _tie = obj;
    _tied = true;
  }

  EventLoop* owner_loop() { return _loop; }
  int fd() const { return _fd; }
  int events() const { return _events; }
  void set_revents(uint32_t recv) { _revents = recv; }
  bool is_disable_all() const { return _events == none_event; }
  bool is_writing() const { return _events & write_event; }
  bool is_reading() const { return _events & read_event; }
  void disable_log_hup() { _log_hup = false; }
  void enable_log_hup() { _log_hup = true; }

  void enable_reading() {
    _events |= read_event;
    update();
  }
  void disable_reading() {
    _events &= ~read_event;
    update();
  }
  void enable_writing() {
    _events |= write_event;
    update();
  }
  void disable_writing() {
    _events &= ~write_event;
    update();
  }
  void disable_all() {
    _events = none_event;
    update();
  }

  void remove();

  string revents_to_string() const;
  string events_to_string() const;

  int index() const { return _index; }
  void set_index(int pos) { _index = pos; }

 private:
  static std::string events_to_string(int fd, int ev);

  void handle_event_with_guard(const Timestamp& receivetime);
  void update();

  static const uint32_t none_event = 0;
  static const uint32_t read_event = POLLIN | POLLPRI;
  ;
  static const uint32_t write_event = POLLOUT;
  ;

  EventLoop* _loop;
  Poller* _poller;
  const int _fd;

  int _index;

  uint32_t _events;
  uint32_t _revents;

  bool _log_hup;

  std::weak_ptr<void> _tie;
  bool _tied;

  bool _event_handling;

  ReadEventCallback _read_callback;
  EventCallback _write_callback;
  EventCallback _close_callback;
  EventCallback _error_callback;
};
}  // namespace net
}  // namespace suduo
#endif