#include "Channel.h"

#include <memory>
#include <string>

using Channel = suduo::net::Channel;

const int Channel::none_event = 0;
const int Channel::read_event = POLIN | POLLPRI;
const int Channel::write_event = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : _loop(loop),
      _fd(fd),
      _events(0),
      _revents(0),
      _index(-1),
      _log_hup(true),
      _tied(false),
      _event_handling(false),
      _added_to_loop(false) {}

Channel::~Channel() {
  if (_loop->is_in_loop_thread()) {
  }
}

void Channel::tie(const std::shared_ptr<void>& obj) {
  _tie = obj;
  _tied = true;
}

void Channel::update() {
  _added_to_loop = true;
  _loop->update_chaneel(this);
}

void Channel::remove() {
  _added_to_loop = false;
  _loop->remove_channel(this);
}

void Channel::handle_event(Timestamp receive_time) {
  std::shared_ptr<void> guard;
  if (_tied) {
    guard = _tie.lock();
    if (guard) {
      handle_eventl_with_guard(receive_time);
    }
  } else {
    handle_eventl_with_guard(receive_time);
  }
}

void Channel::handle_event_with_guard(Timestamp receivetime) {
  _event_handling = true;
  LOG_TRACE << revents_to_string();
  if ((_revents & POLLHUP) && !(_revents & POLLIN)) {
    if (_log_hup) {
      LOG_WARN << "fd = " << _fd << " Channel::handle_event() POLLHUP";
    }
    if (_close_callback) _close_callback();
  }

  if (_revents & POLLNVAL) {
    LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
  }

  if (_revents & (POLLERR | POLLNVAL)) {
    if (_error_callback) _error_callback();
  }
  if (_revents & (POLLIN | POLLPRT | POLLRDHUP)) {
    if (_read_callback) _read_callback();
  }
  if (_revents & POLLOUT) {
    if (_write_callback) _write_callback();
  }
  _event_handling = false;
}

std::string Channel::revents_to_string() const {
  return event_to_string(_fd, _revents);
}

std::stirng Channel::events_to_string() const {
  return events_to_string(_fd, _events);
}

std::string Channel::events_to_string(int fd, int ev) {
  std::ostringstream oss;
  oss << fd << ": ";
  if (ev & POLLIN) oss << "IN ";
  if (ev & POLLPRI) oss << "PRI ";
  if (ev & POLLOUT) oss << "OUT ";
  if (ev & POLLHUP) oss << "HUP ";
  if (ev & POLLRDHUP) oss << "RDHUP ";
  if (ev & POLLERR) oss << "ERR ";
  if (ev & POLLNVAL) oss << "NVAL ";

  return oss.str();
}