#include <stdint.h>
#include <string>
#include "fmt/core.h"
#include "./Tun.h"

#ifdef __APPLE__
  #include "./tun_darwin.cc"
#endif

namespace tunlib {

  bool Tun::open() {
    try {
      fd = openTun(ifName);
    } catch (std::exception& err) {
      fatal << err.what();
      return false;
    }
    info << fmt::format("tunnel [{}] is opened.", ifName);
    return true;
  }
  TunBuffer Tun::read() {
    return readTun(fd, readBuf, MTU);
  }
  uint16_t Tun::write(const uint8_t* buf, uint16_t len) {
    return writeTun(fd, buf, len);
  }

}
