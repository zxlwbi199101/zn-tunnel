#include <stdint.h>
#include <string>
#include <fmt/core.h>

#include <libtun/Tunnel.h>

#ifdef __APPLE__
  #include "./tun_darwin.cc"
#endif

namespace libtun {

  bool Tunnel::open() {
    try {
      fd = openTun(ifName);
    } catch (std::exception& err) {
      fatal << err.what();
      return false;
    }
    info << fmt::format("tunnel [{}] is opened.", ifName);
    return true;
  }
  TunBuffer Tunnel::read() {
    return readTun(fd, readBuf, MTU);
  }
  uint16_t Tunnel::write(const uint8_t* buf, uint16_t len) {
    return writeTun(fd, buf, len);
  }

}
