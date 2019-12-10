cmake_minimum_required(VERSION 3.9)
project(Demo)

add_library (pc  ParseConfig.h ParseConfig.cc)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)

include_directories(/)

add_executable(Demo  test.cc ParseConfig.cc)

target_link_libraries(Demo)