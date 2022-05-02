#ifndef CALLBACKS_H
#define CALLBACKS_H
#include <cstdio>
#include <functional>
#include <memory>

#include "suduo/base/Timestamp.h"
namespace suduo {

template <typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr) {
  return ptr.get();
}

template <typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr) {
  return ptr.get();
}
namespace net {
// All client visible callbacks go here.

class Buffer;
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using HighWaterMarkCallback =
    std::function<void(const TcpConnectionPtr&, size_t)>;

// the data has been read to (buf, len)
using MessageCallback =
    std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;

void defaultConnectionCallback(const TcpConnectionPtr& conn);
void defaultMessageCallback(const TcpConnectionPtr& conn, Buffer* buffer,
                            Timestamp receiveTime);
}  // namespace net
}  // namespace suduo
#endif