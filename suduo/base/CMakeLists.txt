# set(base_SRCS
# Timestamp.cpp
# Mutex.cpp
# Thread.cpp
# CurrentThreadInfo.cpp
# LogStream.cpp
# Logger.cpp
#   )
file(GLOB base_SRCS "*.cpp")

add_library(suduo_base ${base_SRCS})
target_link_libraries(suduo_base pthread rt)

install(TARGETS suduo_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/suduo/base)
add_subdirectory(tests)