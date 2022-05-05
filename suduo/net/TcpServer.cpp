#include "suduo/net/TcpServer.h"

#include <stdio.h>

#include <functional>

#include "suduo/base/Logger.h"
#include "suduo/net/Acceptor.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/EventLoopThreadPool.h"
#include "suduo/net/SocketOpt.h"

using TcpServer = suduo::net::TcpServer;
using namespace suduo::net;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& listen_addr,
                     const std::string& name, Option option)
    : _loop(loop),
      _ip_port(listen_addr.to_Ip_port()),
      _name(name),
      _acceptor(new Acceptor(loop, listen_addr, option == ReusePort)),
      _thread_pool(new EventLoopThreadPool(loop, name)),
      _connection_callback(defaultConnectionCallback),
      _message_callback(defaultMessageCallback),
      _next_conn_id(1) {
  _acceptor->set_new_connection_callback(std::bind(&TcpServer::new_connection,
                                                   this, std::placeholders::_1,
                                                   std::placeholders::_2));
}

TcpServer::~TcpServer() {
  _loop->assert_in_loop_thread();
  LOG_TRACE << "TcpServer::~TcpServer [" << _name << "] destructing";
  for (auto& item : _connections) {
    TcpConnectionPtr conn(item.second);
    item.second.reset();
    conn->get_loop()->run_in_loop(
        std::bind(&TcpConnection::connect_destroyed, conn));
  }
}

void TcpServer::set_thread_num(int threads_num) {
  assert(threads_num >= 0);
  _thread_pool->set_thread_num(threads_num);
}

void TcpServer::start() {
  if (_started == 0) {
    _started = 1;
    thread_pool()->start(_thread_init_callback);
    assert(!_acceptor->listening());
    _loop->run_in_loop(std::bind(&Acceptor::listen, get_pointer(_acceptor)));
  }
}

void TcpServer::new_connection(int sock_fd, const InetAddress& peer_addr) {
  _loop->assert_in_loop_thread();
  EventLoop* ioLoop = _thread_pool->get_next_loop();
  char buf[64];
  snprintf(buf, sizeof(buf), "-%s#%d", _ip_port.c_str(), _next_conn_id);
  ++_next_conn_id;
  string connName = _name + buf;

  LOG_INFO << "TcpServer::newConnection [" << _name << "] - new connection ["
           << connName << "] from " << peer_addr.to_Ip_port();
  InetAddress local_addr(sockets::get_local_addr(sock_fd));
  // FIXME poll with zero timeout to double confirm the new connection
  // FIXME use make_shared if necessary
  TcpConnectionPtr conn(
      new TcpConnection(ioLoop, connName, sock_fd, local_addr, peer_addr));
  _connections[connName] = conn;
  conn->set_connection_callback(_connection_callback);
  conn->set_message_callback(_message_callback);
  conn->set_write_complete_callback(_write_complete_callback);
  conn->set_close_callback(std::bind(&TcpServer::remove_connection, this,
                                     std::placeholders::_1));  // FIXME: unsafe
  ioLoop->run_in_loop(std::bind(&TcpConnection::connect_established, conn));
}

void TcpServer::remove_connection(const TcpConnectionPtr& conn) {
  _loop->run_in_loop(
      std::bind(&TcpServer::remove_connection_in_loop, this, conn));
}

void TcpServer::remove_connection_in_loop(const TcpConnectionPtr& conn) {
  _loop->assert_in_loop_thread();
  LOG_INFO << "TcpServer::removeConnectionInLoop [" << _name
           << "] - connection " << conn->name();
  size_t n = _connections.erase(conn->name());
  (void)n;
  assert(n == 1);
  EventLoop* ioLoop = conn->get_loop();
  ioLoop->queue_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
}
