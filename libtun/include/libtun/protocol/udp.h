#pragma once

#include <stdint.h>
#include <boost/asio/buffer.hpp>
#include <boost/endian.hpp>
#include "./ip4.h"

namespace libtun {
namespace protocol {

  namespace endian = boost::endian;
  using boost::asio::mutable_buffer;

  class Udp {
  public:

    Udp(const Ip4& ip) {
      auto buffer = ip.payload();
      _header = static_cast<Header*>(buffer.data());
      _size = buffer.size();
      _ip = ip;
    }

    Udp(const Udp& other) {
      _size = other._size;
      _header = other._header;
      _ip = other._ip;
    }

    // getters
    uint16_t sourcePort() const {
      return endian::big_to_native(_header->sourcePort);
    }
    uint16_t destPort() const {
      return endian::big_to_native(_header->destPort);
    }
    uint16_t totalLen() const {
      return endian::big_to_native(_header->totalLen);
    }
    uint16_t checksum() const {
      return endian::big_to_native(_header->checksum);
    }
    mutable_buffer payload() {
      return mutable_buffer((uint8_t*)(_header) + 8, _size - 8);
    }

    // setters
    void sourcePort(uint16_t port) {
      _header->sourcePort = endian::native_to_big(port);
    }
    void destPort(uint16_t port) {
      _header->destPort = endian::native_to_big(port);
    }
    void totalLen(uint16_t len) {
      _header->totalLen = endian::native_to_big(len);
    }
    void checksum(uint16_t sum) {
      _header->checksum = endian::native_to_big(sum);
    }

  private:
    struct Header {
      uint16_t sourcePort: 16;
      uint16_t destPort: 16;
      uint16_t totalLen: 16;
      uint16_t checksum: 16;
    };

    Header* _header;
    uint32_t _size;
    Ip4 _ip;
  };

} // namespace protocol
} // namespace libtun
