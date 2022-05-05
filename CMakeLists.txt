cmake_minimum_required(VERSION 3.22)

project(suduo C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

enable_testing()

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Build example if we have
# if(CMAKE_PROJECT_NAME STREQUAL "suduo")
#   option(SUDUO_BUILD_EXAMPLES "Build Suduo examples" ON)
# endif()
# set(CXX_FLAGS
#  -g
#  # -DVALGRIND
#  -DCHECK_PTHREAD_RETURN_VALUE
#  -D_FILE_OFFSET_BITS=64
#  -Wall
#  -Wextra
# #  -Werror
#  -Wconversion
#  -Wno-unused-parameter
#  -Wold-style-cast
#  -Woverloaded-virtual
#  -Wpointer-arith
#  -Wshadow
#  -Wwrite-strings
#  -march=native
#  # -MMD
#  -std=c++11
#  -rdynamic
#  )
 string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
set(CMAKE_CXX_FLAGS_DEBUG "-O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)

find_package(Boost REQUIRED)
find_library(BOOSTTEST_LIBRARY NAMES boost_unit_test_framework)

include_directories(${PROJECT_SOURCE_DIR})

string(TOUPPER ${CMAKE_BUILD_TYPE} BUILD_TYPE)
message(STATUS "CXX_FLAGS = " ${CMAKE_CXX_FLAGS} " " ${CMAKE_CXX_FLAGS_${BUILD_TYPE}})

add_subdirectory(suduo/base)
# add_subdirectory(suduo/net)
add_subdirectory(suduo/net2)