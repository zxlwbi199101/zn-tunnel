#ifndef TEST_UTILS_INCLUDED
#define TEST_UTILS_INCLUDED

#include <boost/asio/buffer.hpp>
#include <boost/filesystem.hpp>
#include <fstream>
#include <string>

namespace fs = boost::filesystem;
using boost::asio::mutable_buffer;

inline mutable_buffer readFile(const std::string& path) {
  auto size = fs::file_size(path);
  auto data = new char[size];

  std::ifstream input(path, std::ifstream::binary);
  input.read(data, size);

  return mutable_buffer(data, size);
}

#endif