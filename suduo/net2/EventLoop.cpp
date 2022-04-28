#include "suduo/net2/EventLoop.h"
using EventLoop = suduo::net::EventLoop;

EventLoop::~EventLoop() {
  if (_running) stop();
}

void EventLoop::loop() {
  while (_running) {
  }
}

void EventLoop::stop() { _running = false; }