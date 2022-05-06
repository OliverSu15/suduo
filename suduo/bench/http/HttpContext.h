// Copyright 2010, Shuo Chen.  All rights reserved.
// http://code.google.com/p/suduo/
//
// Use of this source code is governed by a BSD-style license
// that can be found in the License file.

// Author: Shuo Chen (chenshuo at chenshuo dot com)
//
// This is an internal header file, you should not include this.

#ifndef suduo_NET_HTTP_HTTPCONTEXT_H
#define suduo_NET_HTTP_HTTPCONTEXT_H

#include "suduo/base/copyable.h"
#include "suduo/bench/http/HttpRequest.h"

namespace suduo {
namespace net {

class Buffer;

class HttpContext : public suduo::copyable {
 public:
  enum HttpRequestParseState {
    kExpectRequestLine,
    kExpectHeaders,
    kExpectBody,
    kGotAll,
  };

  HttpContext() : state_(kExpectRequestLine) {}

  // default copy-ctor, dtor and assignment are fine

  // return false if any error
  bool parseRequest(Buffer* buf, Timestamp receiveTime);

  bool gotAll() const { return state_ == kGotAll; }

  void reset() {
    state_ = kExpectRequestLine;
    HttpRequest dummy;
    request_.swap(dummy);
  }

  const HttpRequest& request() const { return request_; }

  HttpRequest& request() { return request_; }

 private:
  bool processRequestLine(const char* begin, const char* end);

  HttpRequestParseState state_;
  HttpRequest request_;
};

}  // namespace net
}  // namespace suduo

#endif  // suduo_NET_HTTP_HTTPCONTEXT_H
