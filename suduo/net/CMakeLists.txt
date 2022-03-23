set(net_SRCS
  )

add_library(suduo_net ${net_SRCS})
target_link_libraries(suduo_net suduo_base)

install(TARGETS suduo_net DESTINATION lib)

set(HEADERS
  )
install(FILES ${HEADERS} DESTINATION include/suduo/net)

#add_subdirectory(tests)