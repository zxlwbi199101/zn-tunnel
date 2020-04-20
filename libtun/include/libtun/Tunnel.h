#ifndef LIBTUN_TUNNEL_INCLUDED
#define LIBTUN_TUNNEL_INCLUDED

#include <stdint.h>
#include <string>
#include <fmt/core.h>
#include <libtun/logger.h>
#include <libtun/BufferPool.h>

#ifdef __APPLE__
  #include "./impl/Tunnel/Tunnel_darwin.h"
#endif

namespace libtun {

  class Tunnel {
  public:
    bool open() {
      try {
        _impl.openTunnel();
      } catch (const std::exception& err) {
        LOG_FATAL << err.what();
        _impl.close();
        return false;
      }
      LOG_INFO << fmt::format("tunnel [{}] is opened.", _impl.ifName);
      return true;
    }

    Buffer read(const Buffer& buf) {
      return _impl.read(buf);
    }

    int write(const Buffer& buf) {
      return _impl.write(buf);
    }

  private:
    impl::TunnelImpl _impl;
  };
}


#endif
