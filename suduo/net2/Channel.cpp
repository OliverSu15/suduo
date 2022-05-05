#include "suduo/net2/Channel.h"

#include <poll.h>

#include "suduo/base/LogStream.h"
#include "suduo/base/Logger.h"
#include "suduo/net/Poller.h"
#include "suduo/net2/EventLoop.h"
using Channel = suduo::net::Channel;
Channel::Channel(const std::unique_ptr<Poller>& poller, int fd)
    : _fd(fd),
      _poller(poller),
      _events(0),
      _revents(0),
      _index(-1),
      _log_hup(true),
      _tied(false),
      _event_handling(false) {}

Channel::~Channel() {
  // if (_loop->is_in_loop_thread()) {
  //   assert(!_loop->has_channel(this));
  // }
}

void Channel::handle_event(const Timestamp& receivetime) {
  std::shared_ptr<void> guard;
  if (_tied) {
    guard = _tie.lock();
    if (guard) {
      handle_event_with_guard(receivetime);
    }
  } else {
    handle_event_with_guard(receivetime);
  }
}

void Channel::handle_event_with_guard(const Timestamp& receivetime) {
  _event_handling = true;
  LOG_TRACE << revents_to_string();
  if ((_revents & POLLHUP) && !(_revents & POLLIN)) {
    if (_log_hup) {
      LOG_WARN << "fd = " << _fd << " Channel::handle_event() POLLHUP";
    }
    if (_close_callback) _close_callback();
  }

  if (_revents & POLLNVAL) {
    LOG_WARN << "fd = " << _fd << " Channel::handle_event() POLLNVAL";
  }

  if (_revents & (POLLERR | POLLNVAL)) {
    if (_error_callback) _error_callback();
  }
  if (_revents & (POLLIN | POLLPRI | POLLRDHUP)) {
    if (_read_callback) _read_callback(receivetime);
  }
  if (_revents & POLLOUT) {
    if (_write_callback) _write_callback();
  }
  _event_handling = false;
}

void Channel::update() { _poller->update_channel(this); }

void Channel::remove() { _poller->remove_channel(this); }

std::string Channel::revents_to_string() const {
  return events_to_string(_fd, _revents);
}

std::string Channel::events_to_string() const {
  return events_to_string(_fd, _events);
}

std::string Channel::events_to_string(int fd, int ev) {
  LogStream oss;
  oss << fd << ": ";
  if (ev & POLLIN) oss << "IN ";
  if (ev & POLLPRI) oss << "PRI ";
  if (ev & POLLOUT) oss << "OUT ";
  if (ev & POLLHUP) oss << "HUP ";
  if (ev & POLLRDHUP) oss << "RDHUP ";
  if (ev & POLLERR) oss << "ERR ";
  if (ev & POLLNVAL) oss << "NVAL ";

  return {oss.buffer().data()};
}
