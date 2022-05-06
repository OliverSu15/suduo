#include <stdio.h>
#include <unistd.h>

#include <atomic>
#include <functional>
#include <utility>

#include "suduo/base/Logger.h"
#include "suduo/base/Thread.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/EventLoopThreadPool.h"
#include "suduo/net/InetAddress.h"
#include "suduo/net/TcpClient.h"

using namespace suduo;
using namespace suduo::net;

class Client;

class Session : noncopyable {
 public:
  Session(EventLoop* loop, const InetAddress& serverAddr, const string& name,
          Client* owner)
      : client_(loop, serverAddr, name),
        owner_(owner),
        bytesRead_(0),
        bytesWritten_(0),
        messagesRead_(0) {
    client_.set_connection_callback(
        std::bind(&Session::onConnection, this, std::placeholders::_1));
    client_.set_message_callback(
        std::bind(&Session::onMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
  }

  void start() { client_.connect(); }

  void stop() { client_.disconnect(); }

  int64_t bytesRead() const { return bytesRead_; }

  int64_t messagesRead() const { return messagesRead_; }

 private:
  void onConnection(const TcpConnectionPtr& conn);

  void onMessage(const TcpConnectionPtr& conn, Buffer* buf, Timestamp) {
    ++messagesRead_;
    bytesRead_ += buf->readable_bytes();
    bytesWritten_ += buf->readable_bytes();
    conn->send(buf);
  }

  TcpClient client_;
  Client* owner_;
  int64_t bytesRead_;
  int64_t bytesWritten_;
  int64_t messagesRead_;
};

class Client : noncopyable {
 public:
  Client(EventLoop* loop, const InetAddress& serverAddr, int blockSize,
         int sessionCount, int timeout, int threadCount)
      : loop_(loop),
        threadPool_(loop, "pingpong-client"),
        sessionCount_(sessionCount),
        timeout_(timeout) {
    loop->run_after(timeout, std::bind(&Client::handleTimeout, this));
    if (threadCount > 1) {
      threadPool_.set_thread_num(threadCount);
    }
    threadPool_.start();

    for (int i = 0; i < blockSize; ++i) {
      message_.push_back(static_cast<char>(i % 128));
    }

    for (int i = 0; i < sessionCount; ++i) {
      char buf[32];
      snprintf(buf, sizeof buf, "C%05d", i);
      Session* session =
          new Session(threadPool_.get_next_loop(), serverAddr, buf, this);
      session->start();
      sessions_.emplace_back(session);
    }
  }

  const string& message() const { return message_; }

  void onConnect() {
    if (++numConnected_ /*incrementAndGet()*/ == sessionCount_) {
      LOG_WARN << "all connected";
    }
  }

  void onDisconnect(const TcpConnectionPtr& conn) {
    if (--numConnected_ /*.decrementAndGet()*/ == 0) {
      LOG_WARN << "all disconnected";

      int64_t totalBytesRead = 0;
      int64_t totalMessagesRead = 0;
      for (const auto& session : sessions_) {
        totalBytesRead += session->bytesRead();
        totalMessagesRead += session->messagesRead();
      }
      LOG_WARN << totalBytesRead << " total bytes read";
      LOG_WARN << totalMessagesRead << " total messages read";
      LOG_WARN << static_cast<double>(totalBytesRead) /
                      static_cast<double>(totalMessagesRead)
               << " average message size";
      LOG_WARN << static_cast<double>(totalBytesRead) / (timeout_ * 1024 * 1024)
               << " MiB/s throughput";
      conn->get_loop()->queue_in_loop(std::bind(&Client::quit, this));
    }
  }

 private:
  void quit() { loop_->queue_in_loop(std::bind(&EventLoop::quit, loop_)); }

  void handleTimeout() {
    LOG_WARN << "stop";
    for (auto& session : sessions_) {
      session->stop();
    }
  }

  EventLoop* loop_;
  EventLoopThreadPool threadPool_;
  int sessionCount_;
  int timeout_;
  std::vector<std::unique_ptr<Session>> sessions_;
  string message_;
  std::atomic_int32_t numConnected_;
};

void Session::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn->set_tcp_no_delay(true);
    conn->send(owner_->message());
    owner_->onConnect();
  } else {
    owner_->onDisconnect(conn);
  }
}

int main(int argc, char* argv[]) {
  if (argc != 7) {
    fprintf(stderr, "Usage: client <host_ip> <port> <threads> <blocksize> ");
    fprintf(stderr, "<sessions> <time>\n");
  } else {
    LOG_INFO << "pid = " << getpid()
             << ", tid = " << Current_thread_info::tid();
    Logger::set_log_level(Logger::WARN);

    const char* ip = argv[1];
    uint16_t port = static_cast<uint16_t>(atoi(argv[2]));
    int threadCount = atoi(argv[3]);
    int blockSize = atoi(argv[4]);
    int sessionCount = atoi(argv[5]);
    int timeout = atoi(argv[6]);

    EventLoop loop;
    InetAddress serverAddr(ip, port);

    Client client(&loop, serverAddr, blockSize, sessionCount, timeout,
                  threadCount);
    loop.loop();
  }
}
