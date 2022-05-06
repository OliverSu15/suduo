#ifndef suduo_EXAMPLES_ASIO_CHAT_CODEC_H
#define suduo_EXAMPLES_ASIO_CHAT_CODEC_H

#include <string>

#include "suduo/base/Logger.h"
#include "suduo/net/Buffer.h"
#include "suduo/net/Endian.h"
#include "suduo/net/TcpConnection.h"

class LengthHeaderCodec : suduo::noncopyable {
 public:
  typedef std::function<void(const suduo::net::TcpConnectionPtr&,
                             const suduo::string& message, suduo::Timestamp)>
      StringMessageCallback;

  explicit LengthHeaderCodec(const StringMessageCallback& cb)
      : messageCallback_(cb) {}

  void onMessage(const suduo::net::TcpConnectionPtr& conn,
                 suduo::net::Buffer* buf, suduo::Timestamp receiveTime) {
    while (buf->readable_bytes() >= kHeaderLen)  // kHeaderLen == 4
    {
      // FIXME: use Buffer::peekInt32()
      const void* data = buf->peek();
      int32_t be32 = *static_cast<const int32_t*>(data);  // SIGBUS
      const int32_t len = suduo::net::sockets::network_to_host_32(be32);
      if (len > 65536 || len < 0) {
        LOG_ERROR << "Invalid length " << len;
        conn->shutdown();  // FIXME: disable reading
        break;
      } else if (buf->readable_bytes() >= len + kHeaderLen) {
        buf->retrieve(kHeaderLen);
        suduo::string message(buf->peek(), len);
        messageCallback_(conn, message, receiveTime);
        buf->retrieve(len);
      } else {
        break;
      }
    }
  }

  // FIXME: TcpConnectionPtr
  void send(suduo::net::TcpConnection* conn, const std::string& message) {
    suduo::net::Buffer buf;
    buf.append(message.data(), message.size());
    int32_t len = static_cast<int32_t>(message.size());
    int32_t be32 = suduo::net::sockets::host_to_network_32(len);
    buf.prepend(&be32, sizeof be32);
    conn->send(&buf);
  }

 private:
  StringMessageCallback messageCallback_;
  const static size_t kHeaderLen = sizeof(int32_t);
};

#endif  // suduo_EXAMPLES_ASIO_CHAT_CODEC_H
