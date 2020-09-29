project(client-cli)
cmake_minimum_required(VERSION 3.15)

set(CMAKE_CXX_STANDARD 20)
add_definitions(-DBOOST_ASIO_DISABLE_CONCEPTS)

if (MSVC)
  add_compile_options(/await)
else()
  add_compile_options(-fcoroutines-ts)
endif()

add_executable(client main.cpp)

# boost (external)
find_package(Boost COMPONENTS regex log_setup log filesystem thread date_time)
target_include_directories(client PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(client ${Boost_LIBRARIES})

# fmt
add_subdirectory(../third_party/fmt build-fmt)
target_link_libraries(client fmt)

# nlohmann-json
include_directories(../third_party/nlohmann-json/include)

# tiny-AES-c
add_subdirectory(../third_party/tiny-AES-c build-tiny-AES-c)
target_link_libraries(client tiny-AES-c)
