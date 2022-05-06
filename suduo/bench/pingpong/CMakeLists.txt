add_executable(pingpong_bench bench.cpp)
target_link_libraries(pingpong_bench suduo_net)

add_executable(pingpong_client client.cpp)
target_link_libraries(pingpong_client suduo_net)

add_executable(pingpong_server server.cpp)
target_link_libraries(pingpong_server suduo_net)