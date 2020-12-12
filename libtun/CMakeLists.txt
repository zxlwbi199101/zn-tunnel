cmake_minimum_required(VERSION 3.15)
project(libtun)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

if (MSVC)
  add_compile_options(/await)
else()
  add_compile_options(-fcoroutines-ts)
endif()

add_subdirectory(test)
add_test(NAME libtest COMMAND test)