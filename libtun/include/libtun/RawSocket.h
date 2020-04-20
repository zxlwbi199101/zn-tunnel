#ifndef LIBTUN_RAW_SOCKET_INCLUDED
#define LIBTUN_RAW_SOCKET_INCLUDED

#include <string>
#include <boost/asio/ip/address_v4.hpp>
#ifdef __APPLE__
  #include "./impl/RawSocket/RawSocket_darwin.h"
#endif

namespace libtun {

  using boost::asio::ip::address_v4;

  class RawSocket {
  public:

    std::string ifName;
    address_v4 ifAddress;
    std::function<void(uint8_t*, uint32_t)> onPacket;

    void start(uint16_t portFrom, uint16_t portTo, const std::string& ifName = "") {
      _useInterface(ifName);
      _impl.open(portFrom, portTo, this->ifName);

      LOG_TRACE << fmt::format(
        "rawsocket open successfully on {}, ip addres: {}",
        this->ifName, ifAddress.to_string()
      );
    }

    void stop() {
      _impl.close();
    }

    int read() {
      int sum = 0, n = 0;
      while ((n = _impl.read(onPacket)) > 1) {
        sum += n;
      }
      return sum;
    }

    int write(uint8_t* data, uint32_t size) {
      return _impl.write(data, size);
    }

  private:
    impl::RawSocketImpl _impl;

    void _useInterface(const std::string& name) {
      auto ifs = _impl.getInterfaces();
      if (ifs.empty()) {
        throw Exception("rowsocket no available IP4 interfaces");
      }

      if (name.empty()) {
        ifName = ifs[0].name;
        ifAddress = ifs[0].address;
        return;
      }

      for (auto i : ifs) {
        if (name == i.name) {
          ifName = i.name;
          ifAddress = i.address;
          return;
        }
      }

      throw Exception(fmt::format("rowsocket ifName '{}' not found", name));
    }
  };

} // namespace libtun

#endif