add_executable(test main.cc BufferPool.cc transmission/Cryptor.cc)

# self include
target_include_directories(test PUBLIC ../include)

# boost (external)
find_package(Boost COMPONENTS regex log_setup log filesystem thread date_time)
target_include_directories(test PUBLIC ${Boost_INCLUDE_DIRS})
target_link_libraries(test ${Boost_LIBRARIES})

# fmt
add_subdirectory(../../third_party/fmt build-fmt)
target_link_libraries(test fmt)

# nlohmann-json
include_directories(../../third_party/nlohmann-json/include)

# tiny-AES-c
add_subdirectory(../../third_party/tiny-AES-c build-tiny-AES-c)
target_link_libraries(test tiny-AES-c)
