#ifndef LIBTUN_PROTOCOL_IP4_INCLUDED
#define LIBTUN_PROTOCOL_IP4_INCLUDED

#include <stdint.h>
#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/network_v4.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/endian/conversion.hpp>

namespace libtun {
namespace protocol {

  namespace endian = boost::endian;
  using boost::asio::mutable_buffer;

  class Ip4 {
  public:

    typedef boost::asio::ip::address_v4 Address;
    typedef boost::asio::ip::network_v4 Network;

    enum Protocol: uint8_t {
      ICMP = 1,
      IGMP = 2,
      TCP = 6,
      UDP = 17,
      IGRP = 88,
      OSPF = 89,
    };

    enum Flag: uint8_t {
      NONE = 0,
      DONT_FRAGMENT = 2,
      MORE_FRAGMENT = 1,
    };

    Ip4() {}

    Ip4(mutable_buffer buffer) {
      _header = (Header*)buffer.data();
      _size = buffer.size();
    }

    Ip4(const Ip4& other) {
      _header = other._header;
      _size = other._size;
    }

    // getters
    uint8_t version() const {
      return (_header->ver_ihl & 0xf0) >> 4;
    }
    uint8_t headerLen() const {
      return _header->ver_ihl & 0xf;
    }
    u_int8_t serviceType() const {
      return _header->serviceType;
    }
    uint16_t totalLen() const {
      return endian::big_to_native(_header->totalLen);
    }
    uint16_t id() const {
      return endian::big_to_native(_header->id);
    }
    Flag flag() const {
      return static_cast<Flag>(endian::big_to_native(_header->flags_offset) >> 13);
    }
    uint16_t offset() const {
      return endian::big_to_native(_header->flags_offset) & 0x1fff;
    }
    uint8_t ttl() const {
      return _header->ttl;
    }
    Protocol protocol() const {
      return static_cast<Protocol>(_header->protocol);
    }
    uint16_t checksum() const {
      return endian::big_to_native(_header->checksum);
    }
    Address sourceIP() const {
      return Address(endian::big_to_native(_header->sourceIP));
    }
    Address destIP() const {
      return Address(endian::big_to_native(_header->destIP));
    }
    mutable_buffer payload() const {
      return mutable_buffer(
        (uint8_t*)_header + headerLen() * 4,
        totalLen() - headerLen() * 4
      );
    }
    mutable_buffer internalBuffer() const {
      return mutable_buffer(_header, _size);
    }

    // setters
    void checksum(uint16_t sum) {
      _header->checksum = endian::native_to_big(sum);
    }
    void sourceIP(Address ip) {
      _header->sourceIP = endian::native_to_big(ip.to_uint());
    }
    void destIP(Address ip) {
      _header->destIP = endian::native_to_big(ip.to_uint());
    }

    // mutates
    uint16_t calculateChecksum() {
      uint8_t* data = (uint8_t*)_header;
      uint32_t sum = 0;
      uint8_t len = headerLen() * 4;

      for (int i = 0; i < len; i += 2) {
        if (i != 10) {
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
      uint8_t ver_ihl;
      uint8_t serviceType;
      uint16_t totalLen;
      uint16_t id;
      uint16_t flags_offset;
      uint8_t ttl;
      uint8_t protocol;
      uint16_t checksum;
      uint32_t sourceIP;
      uint32_t destIP;
    };

    Header* _header;
    uint32_t _size;
  };

} // namespace protocol
} // namespace libtun

#endif