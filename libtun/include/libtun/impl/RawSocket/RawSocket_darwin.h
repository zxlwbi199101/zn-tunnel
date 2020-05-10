#ifndef LIBTUN_IMPL_RAWSOCKET_DARWIN_INCLUDED
#define LIBTUN_IMPL_RAWSOCKET_DARWIN_INCLUDED

#include <net/if.h>
#include <net/bpf.h>
#include <net/ethernet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <ifaddrs.h>

#include <vector>
#include <functional>
#include <fmt/core.h>
#include <libtun/logger.h>
#include <libtun/protocol.h>
#include <libtun/Exception.h>
#include <libtun/BufferPool.h>
#include <libtun/RawSocket.h>
#include "./InterfaceInfo.h"

namespace libtun {
namespace impl {

  const int ETHER_HEADER_LEN = sizeof(u_char) * 12 + sizeof(u_short);

  class RawSocketImpl {
  public:

    std::vector<InterfaceInfo> getInterfaces() {
      ifaddrs* ifaddr;
      if (getifaddrs(&ifaddr) == -1) {
        throw libtun::Exception(fmt::format("raw socket getifaddrs failed: ", strerror(errno)));
      }

      char host[NI_MAXHOST];
      std::vector<InterfaceInfo> ifs;

      for (auto cur = ifaddr; cur; cur = cur->ifa_next) {
        if (
          !cur->ifa_addr ||
          cur->ifa_addr->sa_family != AF_INET ||
          !(cur->ifa_flags & IFF_UP) ||
          (cur->ifa_flags & IFF_LOOPBACK)
        ) {
          continue;
        }

        if (getnameinfo(cur->ifa_addr, sizeof(sockaddr_in), host, NI_MAXHOST, NULL, 0, NI_NUMERICHOST) == -1) {
          throw libtun::Exception(fmt::format("raw socket getnameinfo failed: ", strerror(errno)));
        }
        ifs.push_back({
          .name = std::string(cur->ifa_name),
          .address = boost::asio::ip::make_address_v4(host),
        });
      }

      freeifaddrs(ifaddr);
      return ifs;
    }

    void open(uint16_t portFrom, uint16_t portTo, const std::string& ifName) {
      for (uint8_t i = 0; i < 99 && _fd == -1; i++) {
        _fd = ::open(fmt::format("/dev/bpf{}", i).c_str(), O_RDWR);
      }
      if (_fd == -1) {
        throw Exception(fmt::format("cannot open /dev/bpf: {}", strerror(errno)));
      }

      ifreq boundIf = { .ifr_name = *(ifName.data()) };
      int enabled = 1, disabled = 0;

      if (
        ioctl(_fd, BIOCSETIF, &boundIf) == -1 &&
        ioctl(_fd, BIOCIMMEDIATE, &enabled) == -1 &&
        ioctl(_fd, BIOCGBLEN, &_bufLen) == -1 &&
        ioctl(_fd, BIOCSSEESENT, &disabled) == -1
      ) {
        throw Exception(fmt::format("bpf ioctl error: {}", strerror(errno)));
      }

      _buf = new uint8_t[_bufLen];
    }

    void close() {
      ::close(_fd);
      _fd = -1;
      delete[] _buf;
    }

    int read() {
      if (!opened()) {
        throw Exception("raw socket is not open");
      }
      return (_readableLen = ::read(_fd, _buf, _bufLen));
    }

    int write(uint8_t* data, uint32_t size) {
      return ::write(_fd, data, size);
    }

    bool opened() {
      return _fd > 0;
    }
    bool consumable() {
      return _readableLen > 0;
    }

    void consume(std::function<void(uint8_t*, uint32_t)> onPacket) {
      if (!consumable()) return;

      for (auto cur = _buf; cur < _buf + _readableLen;) {
        bpf_hdr* bpfHeader = (bpf_hdr*)cur;
        ether_header* etherHeader = (ether_header*)(cur + bpfHeader->bh_hdrlen);
        auto ipData = cur + bpfHeader->bh_hdrlen + ETHER_HEADER_LEN;
        auto ipSize = bpfHeader->bh_caplen - ETHER_HEADER_LEN;

        uint16_t srcPort, destPort;
        libtun::protocol::Ip4 packet(ipData, ipSize);
        if (
          packet.protocol() == libtun::protocol::Ip4::Protocol::TCP ||
          packet.protocol() == libtun::protocol::Ip4::Protocol::UDP
        ) {
          libtun::protocol::Tcp tcp(packet);
          LOG_TRACE << fmt::format(
            "{}:{} -> {}:{}, protocol: {}, len: {}",
            packet.sourceIP().to_string(),
            tcp.sourcePort(),
            packet.destIP().to_string(),
            tcp.destPort(),
            packet.protocol() == libtun::protocol::Ip4::Protocol::TCP ? "TCP" : "UDP",
            packet.totalLen()
          );
        } else {
          LOG_TRACE << fmt::format(
            "{} -> {}, protocol: {}, len: {}",
            packet.sourceIP().to_string(),
            packet.destIP().to_string(),
            packet.protocol(),
            packet.totalLen()
          );
        }

        onPacket(ipData, ipSize);
        cur += BPF_WORDALIGN(bpfHeader->bh_hdrlen + bpfHeader->bh_caplen);
      }
      _readableLen = 0;
    }

  private:
    int _fd = -1;
    int _bufLen = 0;
    uint8_t* _buf;
    int _readableLen = 0;

  };

} // namespace impl
} // namespace libtun

#endif