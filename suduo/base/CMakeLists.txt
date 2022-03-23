set(base_SRCS

  )

add_library(suduo_base ${base_SRCS})
target_link_libraries(suduo_base pthread rt)

install(TARGETS suduo_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/suduo/base)
#add_subdirectory(tests)