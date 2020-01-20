#ifndef LIBTUN_TUNNEL_INCLUDED
#define LIBTUN_TUNNEL_INCLUDED

#include <stdint.h>
#include <string>
#include <boost/asio/buffer.hpp>
#include <fmt/core.h>
#include <libtun/logger.h>

namespace libtun {

  using boost::asio::const_buffer;
  using boost::asio::mutable_buffer;

  const uint16_t MTU = 1500;

  class Tunnel {
  public:
    std::string ifName;
    uint8_t readBuffer[MTU];

    bool open() {
      try {
        _openTun();
      } catch (const std::exception& err) {
        fd = -1;
        fatal << err.what();
        return false;
      }
      info << fmt::format("tunnel [{}] is opened.", ifName);
      return true;
    };

    bool isReady() {
      return fd > 0;
    }

    mutable_buffer read(mutable_buffer buffer);
    uint16_t read();
    uint16_t write(const const_buffer& buffer);

  private:
    int fd = -1;
    void _openTun();
  };
}

#ifdef __APPLE__
  #include "./impl/Tunnel/tun_darwin.h"
#endif

#endif
