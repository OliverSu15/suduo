#ifndef SUDUO_NET_CALLBACKS_H
#define SUDUO_NET_CALLBACKS_H
#include <functional>
#include <memory>

#include "suduo/base/Timestamp.h"
namespace suduo {
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
}  // namespace net
}  // namespace suduo
#endif