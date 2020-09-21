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

add_executable(client main.cpp)

# boost (external)
find_package(Boost COMPONENTS regex system)
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries (client ${Boost_LIBRARIES})

# fmt
add_subdirectory(../third_party/fmt build-fmt)
# include_directories("../third_party/fmt/include")
target_link_libraries(client fmt)

# nlohmann-json
include_directories(../third_party/nlohmann-json/include)

# tiny-AES-c
# add_subdirectory(../third_party/tiny-AES-c build-tiny-AES-c)
# include_directories(../third_party/tiny-AES-c/include)
# target_link_libraries(client tiny-aes-c)
