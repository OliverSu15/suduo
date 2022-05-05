file(GLOB net_SRCS "*.cpp" "poller/*.cpp")
add_library(suduo_net ${net_SRCS})
target_link_libraries(suduo_net suduo_base)
install(TARGETS suduo_net DESTINATION lib)

file(GLOB HEADERS "*.h" "poller/*.h")
install(FILES ${HEADERS} DESTINATION include/suduo/net)

add_subdirectory(test)