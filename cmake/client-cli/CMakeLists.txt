project(client-cli)
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)

if (MSVC)
  add_compile_options(/await)
  add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)
else()
  add_compile_options(-fcoroutines-ts)
  add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)
  add_definitions(-DBOOST_LOG_DYN_LINK)
endif()

find_package(Boost COMPONENTS regex system)

add_executable(client main.cpp)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries (client ${Boost_LIBRARIES})

