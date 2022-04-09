#ifndef CHANNEL_H
#define CHANEEL_H
#include <functional>
#include <memory>
#include <utility>

#include "suduo/base/Timestamp.h"
#include "suduo/base/noncopyable.h"
namespace suduo {
namespace net {
class EventLoop;
class Channel : noncopyable {
 public:
  using EventCallback = std::function<void()>;
  using ReadEventCallback = std::function<void(Timestamp)>;
  Channel(EventLoop* loop, int fd);
  ~Channel();

  void handle_event(Timestamp receive_time);

  void set_read_callback(ReadEventCallback cb) {
    _read_callback = std::move(cb);
  }
  void set_write_callback(EventCallback cb) { _write_callback = std::move(cb); }
  void set_close_callback(EventCallback cb) { _close_callback = std::move(cb); }
  void set_error_callback(EventCallback cb) { _error_callback = std::move(cb); }

  void tie(const std::shared_ptr<void>&);

  int fd() const { return _fd; }
  int event_size() const { return _events; }
  int set_revents(int revt) { _revents = revt; }
  bool is_none_event() const { return _events == none_event; }

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

  bool is_writing() const { return _events & write_event; }
  bool is_reading() const { return _events & read_event; }

  int index() { return _index; }
  void set_index(int pos) { _index = pos; }

  std::string revents_to_string() const;
  std::string events_to_string() const;

  void do_not_log_hup() { _log_hup = false; }
  EventLoop* owner_loop() { return _loop; }
  void remove();

 private:
  static std::string event_to_string(int fd, int ev);

  void update();
  void handle_eventl_with_guard(Timestamp receive_time);
  static const int none_event;
  static const int read_event;
  static const int write_event;

  EventLoop* _loop;
  const int _fd;
  int _events;
  int _revents;
  int _index;
  int _log_hup;

  std::weak_ptr<void> _tie;
  bool _tied;
  bool _event_handling;
  bool _added_to_loop;

  ReadEventCallback _read_callback;
  EventCallback _write_callback;
  EventCallback _close_callback;
  EventCallback _error_callback;
};
}  // namespace net
}  // namespace suduo
#endif