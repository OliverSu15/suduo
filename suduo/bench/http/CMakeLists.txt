set(http_SRCS
  HttpServer.cpp
  HttpResponse.cpp
  HttpContext.cpp
  )

add_library(suduo_http ${http_SRCS})
target_link_libraries(suduo_http suduo_net)

install(TARGETS suduo_http DESTINATION lib)
set(HEADERS
  HttpContext.h
  HttpRequest.h
  HttpResponse.h
  HttpServer.h
  )
install(FILES ${HEADERS} DESTINATION include/suduo/bench/http)

# if(suduo_BUILD_EXAMPLES)
add_executable(httpserver_test tests/HttpServer_test.cpp)
target_link_libraries(httpserver_test suduo_http)

if(BOOSTTEST_LIBRARY)
add_executable(httprequest_unittest tests/HttpRequest_unittest.cpp)
target_link_libraries(httprequest_unittest suduo_http boost_unit_test_framework)
endif()

# endif()

# add_subdirectory(tests)
