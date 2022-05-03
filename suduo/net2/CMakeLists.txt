file(GLOB net_SRCS "*.cpp" "poller/*.cpp")
add_library(suduo_net2 ${net_SRCS})
target_link_libraries(suduo_net2 suduo_base)
install(TARGETS suduo_net2 DESTINATION lib)

file(GLOB HEADERS "*.h" "poller/*.h")
install(FILES ${HEADERS} DESTINATION include/suduo/net2)

# add_subdirectory(test)