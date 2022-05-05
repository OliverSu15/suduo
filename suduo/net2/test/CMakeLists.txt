add_executable(channel_test Channel_test.cpp)
target_link_libraries(channel_test suduo_net2)

add_executable(echoserver_unittest EchoServer_unittest.cpp)
target_link_libraries(echoserver_unittest suduo_net2)

add_executable(echoclient_unittest EchoClient_unittest.cpp)
target_link_libraries(echoclient_unittest suduo_net2)

add_executable(eventloop_unittest EventLoop_unittest.cpp)
target_link_libraries(eventloop_unittest suduo_net2)

add_executable(eventloopthread_unittest EventLoopThread_unittest.cpp)
target_link_libraries(eventloopthread_unittest suduo_net2)

add_executable(eventloopthreadpool_unittest EventLoopThreadPool_unittest.cpp)
target_link_libraries(eventloopthreadpool_unittest suduo_net2)

if(BOOSTTEST_LIBRARY)
add_executable(buffer_unittest Buffer_unittest.cpp)
target_link_libraries(buffer_unittest suduo_net2 boost_unit_test_framework)
add_test(NAME buffer_unittest COMMAND buffer_unittest)

add_executable(inetaddress_unittest InetAddress_unittest.cpp)
target_link_libraries(inetaddress_unittest suduo_net2 boost_unit_test_framework)
add_test(NAME inetaddress_unittest COMMAND inetaddress_unittest)

# if(ZLIB_FOUND)
#   add_executable(zlibstream_unittest ZlibStream_unittest.cc)
#   target_link_libraries(zlibstream_unittest suduo_net2 boost_unit_test_framework z)
#   # set_target_properties(zlibstream_unittest PROPERTIES COMPILE_FLAGS "-std=c++0x")
# endif()

endif()

add_executable(tcpclient_reg1 TcpClient_reg1.cpp)
target_link_libraries(tcpclient_reg1 suduo_net2)

add_executable(tcpclient_reg2 TcpClient_reg2.cpp)
target_link_libraries(tcpclient_reg2 suduo_net2)

add_executable(tcpclient_reg3 TcpClient_reg3.cpp)
target_link_libraries(tcpclient_reg3 suduo_net2)

add_executable(timerqueue_unittest TimerQueue_unittest.cpp)
target_link_libraries(timerqueue_unittest suduo_net2)
add_test(NAME timerqueue_unittest COMMAND timerqueue_unittest)

