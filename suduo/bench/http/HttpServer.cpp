// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/suduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//

#include "suduo/bench/http/HttpServer.h"

#include "suduo/base/Logger.h"
#include "suduo/bench/http/HttpContext.h"
#include "suduo/bench/http/HttpRequest.h"
#include "suduo/bench/http/HttpResponse.h"

using namespace suduo;
using namespace suduo::net;

namespace suduo {
namespace net {
namespace detail {

void defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
  resp->setStatusCode(HttpResponse::k404NotFound);
  resp->setStatusMessage("Not Found");
  resp->setCloseConnection(true);
}

}  // namespace detail
}  // namespace net
}  // namespace suduo

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr,
                       const string& name, TcpServer::Option option)
    : server_(loop, listenAddr, name, option),
      httpCallback_(detail::defaultHttpCallback) {
  server_.set_connection_callback(
      std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
  server_.set_message_callback(
      std::bind(&HttpServer::onMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start() {
  LOG_WARN << "HttpServer[" << server_.name() << "] starts listening on "
           << server_.ip_port();
  server_.start();
}

void HttpServer::onConnection(const TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn->set_context(HttpContext());
  }
}

void HttpServer::onMessage(const TcpConnectionPtr& conn, Buffer* buf,
                           Timestamp receiveTime) {
  HttpContext* context =
      std::any_cast<HttpContext>(conn->get_mutable_context());

  if (!context->parseRequest(buf, receiveTime)) {
    conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
    conn->shutdown();
  }

  if (context->gotAll()) {
    onRequest(conn, context->request());
    context->reset();
  }
}

void HttpServer::onRequest(const TcpConnectionPtr& conn,
                           const HttpRequest& req) {
  const string& connection = req.getHeader("Connection");
  bool close =
      connection == "close" ||
      (req.getVersion() == HttpRequest::kHttp10 && connection != "Keep-Alive");
  HttpResponse response(close);
  httpCallback_(req, &response);
  Buffer buf;
  response.appendToBuffer(&buf);
  conn->send(&buf);
  if (response.closeConnection()) {
    conn->shutdown();
  }
}
