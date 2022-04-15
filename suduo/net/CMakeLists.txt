set(net_SRCS
  Acceptor.cpp
  Buffer.cpp
  Channel.cpp
  Connector.cpp
  EventLoop.cpp
  EventLoopThread.cpp
  EventLoopThreadPool.cpp
  InetAddress.cpp
  Poller.cpp
  poller/DefaultPoller.cpp
  poller/EPollPoller.cpp
  poller/PollPoller.cpp
  Socket.cpp
  SocketOpt.cpp
  TcpClient.cpp
  TcpConnection.cpp
  TcpServer.cpp
  Timer.cpp
  TimerQueue.cpp
  )
# file(GLOB net_SRCS "*.cpp")
add_library(suduo_net ${net_SRCS})
target_link_libraries(suduo_net suduo_base)

install(TARGETS suduo_net DESTINATION lib)

set(HEADERS
  Buffer.h
  Callbacks.h
  Channel.h
  Endian.h
  EventLoop.h
  EventLoopThread.h
  EventLoopThreadPool.h
  InetAddress.h
  TcpClient.h
  TcpConnection.h
  TcpServer.h
  TimerId.h
  )
install(FILES ${HEADERS} DESTINATION include/suduo/net)
# if(MUDUO_BUILD_EXAMPLES)
#   add_subdirectory(tests)
# endif()
#add_subdirectory(tests)