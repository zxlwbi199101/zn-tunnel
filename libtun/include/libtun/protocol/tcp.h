#pragma once

#include <stdint.h>
#include <boost/asio/buffer.hpp>
#include <boost/endian.hpp>
#include "./ip4.h"

namespace libtun {
namespace protocol {

  namespace endian = boost::endian;
  using boost::asio::mutable_buffer;

  class Tcp {
  public:

    Tcp(const Ip4& ip) {
      auto buffer = ip.payload();
      _header = static_cast<Header*>(buffer.data());
      _size = buffer.size();
      _ip = ip;
    }

    Tcp(const Tcp& other) {
      _size = other._size;
      _header = other._header;
      _ip = other._ip;
    }

    // getters
    uint16_t sourcePort() {
      return endian::big_to_native(_header->sourcePort);
    }
    uint16_t destPort() {
      return endian::big_to_native(_header->destPort);
    }
    uint32_t sequence() {
      return endian::big_to_native(_header->sequence);
    }
    uint32_t acknowledgment() {
      return endian::big_to_native(_header->acknowledgment);
    }
    uint32_t headerLen() {
      return (_header->hlen_reserve & 0xf0) >> 4;
    }
    bool CWR() { return _header->flags & 0b10000000; }
    bool ECE() { return _header->flags & 0b01000000; }
    bool URG() { return _header->flags & 0b00100000; }
    bool ACK() { return _header->flags & 0b00010000; }
    bool PSH() { return _header->flags & 0b00001000; }
    bool RST() { return _header->flags & 0b00000100; }
    bool SYN() { return _header->flags & 0b00000010; }
    bool FIN() { return _header->flags & 0b00000001; }
    uint16_t window() {
      return endian::big_to_native(_header->window);
    }
    uint16_t checksum() {
      return endian::big_to_native(_header->checksum);
    }
    uint16_t urgentPointer() {
      return endian::big_to_native(_header->urgentPointer);
    }
    mutable_buffer payload() {
      return mutable_buffer(
        (uint8_t*)(_header) + headerLen() * 4,
        _size - headerLen() * 4
      );
    }

    // setters
    void sourcePort(uint16_t port) {
      _header->sourcePort = endian::native_to_big(port);
    }
    void destPort(uint16_t port) {
      _header->destPort = endian::native_to_big(port);
    }
    void checksum(uint16_t sum) {
      _header->checksum = endian::native_to_big(sum);
    }

    // mutates
    uint16_t calculateChecksum() {
      uint8_t* data = (uint8_t*)_header;
      uint32_t sum = 0;
      auto sourceIP = _ip.sourceIP().to_bytes();
      auto destIP = _ip.destIP().to_bytes();

      sum += ((uint16_t)sourceIP[0] << 8) + sourceIP[1] + ((uint16_t)sourceIP[2] << 8) + sourceIP[3];
      sum += ((uint16_t)destIP[0] << 8) + destIP[1] + ((uint16_t)destIP[2] << 8) + destIP[3];
      sum += static_cast<uint8_t>(_ip.protocol());
      sum += _size;

      for (int i = 0; i < _size; i += 2) {
        if (i != 16) {
          sum += ((uint16_t)data[i] << 8) + data[i + 1];
        }
      }

      while (sum >> 16) {
        sum = (sum & 0xffff) + (sum >> 16);
      }
      return ~sum;
    }

    void calculateChecksumInplace() {
      checksum(calculateChecksum());
    }


  private:

    struct Header {
      uint16_t sourcePort;
      uint16_t destPort;
      uint32_t sequence;
      uint32_t acknowledgment;
      uint8_t hlen_reserve;
      uint8_t flags;
      uint16_t window;
      uint16_t checksum;
      uint16_t urgentPointer;
    };

    Header* _header;
    uint32_t _size;
    Ip4 _ip;

  };

} // namespace protocol
} // namespace libtun
