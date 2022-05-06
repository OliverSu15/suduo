#include "suduo/net/TcpClient.h"

#include <stdio.h>

#include <functional>

#include "suduo/base/Logger.h"
#include "suduo/net/Connector.h"
#include "suduo/net/EventLoop.h"
#include "suduo/net/SocketOpt.h"

using TcpClient = suduo::net::TcpClient;
using namespace suduo;
using namespace suduo::net;
namespace suduo {
namespace net {
namespace detail {
void remove_connection(EventLoop* loop, const TcpConnectionPtr& conn) {
  loop->queue_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
}

void remove_connector(const ConnectorPtr& connector) {}
}  // namespace detail
}  // namespace net
}  // namespace suduo

TcpClient::TcpClient(EventLoop* loop, const InetAddress& server_addr,
                     const std::string& name)
    : _loop(loop),
      _connector(new Connector(loop, server_addr)),
      _name(name),
      _connection_callback(defaultConnectionCallback),
      _message_callback(defaultMessageCallback),
      _retry(false),
      _connect(true),
      _next_conn_id(1) {
  _connector->set_new_connection_callback(
      std::bind(&TcpClient::new_connection, this, std::placeholders::_1));
  // FIXME setConnectFailedCallback
  LOG_INFO << "TcpClient::TcpClient[" << _name << "] - connector "
           << get_pointer(_connector);
}

TcpClient::~TcpClient() {
  LOG_INFO << "TcpClient::~TcpClient[" << _name << "] - connector "
           << get_pointer(_connector);
  TcpConnectionPtr conn;
  bool unique = false;
  {
    MutexLockGuard lock(_mutex);
    unique = _connection.unique();
    conn = _connection;
  }
  if (conn) {
    assert(_loop == conn->get_loop());
    CloseCallback cb =
        std::bind(&detail::remove_connection, _loop, std::placeholders::_1);
    _loop->run_in_loop(std::bind(&TcpConnection::set_close_callback, conn, cb));
    if (unique) {
      conn->force_close();
    }
  } else {
    _connector->stop();
    _loop->run_after(1, std::bind(&detail::remove_connector, _connector));
  }
}

void TcpClient::connect() {
  LOG_INFO << "TcpClient::connect[" << _name << "] - connecting to "
           << _connector->server_address().to_Ip_port();

  _connect = true;
  _connector->start();
}

void TcpClient::disconnect() {
  _connect = false;
  {
    MutexLockGuard lock(_mutex);
    if (_connection) {
      _connection->shutdown();
    }
  }
}

void TcpClient::stop() {
  _connect = false;
  _connector->stop();
}

void TcpClient::new_connection(int sock_fd) {
  _loop->assert_in_loop_thread();
  InetAddress peer_addr(sockets::get_peer_addr(sock_fd));
  char buf[32];
  snprintf(buf, sizeof(buf), ":%s#%d", peer_addr.to_Ip_port().c_str(),
           _next_conn_id);
  ++_next_conn_id;
  std::string conn_name = _name + buf;

  InetAddress local_addr(sockets::get_local_addr(sock_fd));
  TcpConnectionPtr conn(
      new TcpConnection(_loop, conn_name, sock_fd, local_addr, peer_addr));
  conn->set_connection_callback(_connection_callback);
  conn->set_message_callback(_message_callback);
  conn->set_write_complete_callback(_write_complete_callback);
  conn->set_close_callback(
      std::bind(&TcpClient::remove_connection, this, std::placeholders::_1));
  {
    MutexLockGuard lock(_mutex);
    _connection = conn;
  }
  conn->connect_established();
}

void TcpClient::remove_connection(const TcpConnectionPtr& conn) {
  _loop->assert_in_loop_thread();
  assert(_loop == conn->get_loop());
  {
    MutexLockGuard lock(_mutex);
    assert(_connection == conn);
    _connection.reset();
  }
  _loop->queue_in_loop(std::bind(&TcpConnection::connect_destroyed, conn));
  if (_retry && _connect) {
    LOG_INFO << "TcpClient::connect[" << _name << "] - Reconnecting to "
             << _connector->server_address().to_Ip_port();
    _connector->restart();
  }
}