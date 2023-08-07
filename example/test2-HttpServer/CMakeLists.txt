cmake_minimum_required (VERSION 3.0)
project(HttpServer)

# 加载子目录
set(HTTP_SRCS
  HttpServer.cc
  HttpResponse.cc
  HttpContext.cc
  main.cc
)

# include_directories(${CMAKE_SOURCE_DIR}/../../src)  # 这里指定 src 目录的路径
include_directories(/usr/include/mymuduo)


set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)

add_executable(HttpServer ${HTTP_SRCS})

target_link_libraries(HttpServer mymuduo pthread)
