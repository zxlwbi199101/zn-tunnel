#pragma once

#include <string>
#include <thread>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address_v4.hpp>
#ifdef __APPLE__
  #include "./impl/RawSocket/RawSocket_darwin.h"
#endif

namespace libtun {

  using boost::asio::io_context;
  using boost::asio::ip::address_v4;

  class RawSocket {
  public:

    std::string ifName;
    address_v4 ifAddress;
    std::function<void(uint8_t*, uint32_t)> onPacket;

    void start(io_context* context, uint16_t portFrom, uint16_t portTo, const std::string& ifName = "") {
      _context = context;
      _useInterface(ifName);
      _impl.open(portFrom, portTo, this->ifName);

      LOG_TRACE << fmt::format(
        "raw socket open successfully on: {}, ip addres: {}",
        this->ifName, ifAddress.to_string()
      );

      _reading = true;
      _thread = std::thread(std::bind(&RawSocket::_startRead, this));
    }

    void stop() {
      _reading = false;
      _impl.close();
    }

    int write(uint8_t* data, uint32_t size) {
      return _impl.write(data, size);
    }

  private:
    impl::RawSocketImpl _impl;
    io_context* _context;
    bool _reading = false;
    std::thread _thread;
    std::mutex _mutex;
    std::condition_variable _cv;

    void _startRead() {
      while (_reading) {
        std::unique_lock<std::mutex> locker(_mutex);
        _impl.read();
        if (!_reading) return;

        boost::asio::post(*_context, [&]() {
          std::unique_lock<std::mutex> locker(_mutex);
          _impl.consume(onPacket);
          _cv.notify_one();
        });

        _cv.wait(locker, [&]() { return _impl.consumable(); });
      }
    }

    void _useInterface(const std::string& name) {
      auto ifs = _impl.getInterfaces();
      if (ifs.empty()) {
        throw Exception("raw socket no available IP4 interfaces");
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

      throw Exception(fmt::format("raw socket ifName '{}' not found", name));
    }
  };

} // namespace libtun
