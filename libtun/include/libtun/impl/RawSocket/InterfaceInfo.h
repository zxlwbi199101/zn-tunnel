#pragma once

#include <string>
#include <boost/asio/ip/address_v4.hpp>

namespace libtun {
namespace impl {

  struct InterfaceInfo {
    std::string name;
    boost::asio::ip::address_v4 address;
  };

} // namespace impl
} // namespace libtun
