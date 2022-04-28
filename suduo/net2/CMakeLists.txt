file(GLOB net_SRCS "*.cpp")
add_library(suduo_net2 ${net_SRCS})
target_link_libraries(suduo_net2 suduo_base)
install(TARGETS suduo_net2 DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/suduo/net)

# add_subdirectory(test)