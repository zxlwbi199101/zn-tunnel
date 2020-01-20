#include <vector>
#include <boost/asio.hpp>
#include <net/bpf.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <libtun/logger.h>
#include <libtun/protocol/ip4.h>

namespace asio = boost::asio;
using boost::asio::mutable_buffer;
using socket_addr_type = boost::asio::detail::socket_addr_type;

int main() {
  try {
    libtun::BinLogger ipdump("/Users/zxl/Desktop/ip.dump");

    int fd = -1;
    for (uint8_t i = 0; i < 99; i++) {
      std::string bpfName = "/dev/bpf" + std::to_string(i);
      if ((fd = open(bpfName.c_str(), O_RDWR)) != -1) {
        break;
      }
    }

    struct ifreq boundIf = { .ifr_name = "en0" };
    int bufLen = 1;
    ioctl(fd, BIOCSETIF, &boundIf);
    ioctl(fd, BIOCIMMEDIATE, &bufLen);
    ioctl(fd, BIOCGBLEN, &bufLen);

    uint8_t* buffer = new uint8_t[bufLen];
    struct bpf_hdr* bpfHeader;
    struct ether_header* etherHeader;
    const uint8_t etherHLen = sizeof(u_char) * 12 + sizeof(u_short);

    while (1) {
      int n = read(fd, buffer, bufLen);
      for (auto cur = buffer; cur < buffer + n;) {
        bpfHeader = (bpf_hdr*)cur;
        etherHeader = (ether_header*)(cur + bpfHeader->bh_hdrlen);

        if (etherHeader->ether_type == 8) {
          auto ipBuffer = mutable_buffer(
            cur + bpfHeader->bh_hdrlen + etherHLen,
            bpfHeader->bh_caplen - etherHLen
          );
          libtun::protocol::IP4 packet(ipBuffer);
          std::string meta = fmt::format(
            "IP_PACKET: {} -> {}, protocol: {}, len: {}, bufferLen: {}, flag: {} END",
            packet.sourceIP().to_string(),
            packet.destIP().to_string(),
            packet.protocol(),
            packet.totalLen(),
            ipBuffer.size(),
            packet.flag()
          );
          ipdump.dump(ipBuffer.data(), ipBuffer.size());
          ipdump.dump(meta.c_str(), meta.size());
          trace << meta;
        }

        cur += BPF_WORDALIGN(bpfHeader->bh_hdrlen + bpfHeader->bh_caplen);
      }
    }




    // int fd = socket (PF_INET, SOCK_RAW, 255);
    // int one = 1;
    // const int *val = &one;
    // setsockopt(fd, IPPROTO_IP, IP_HDRINCL, val, sizeof(one));

    // int idx = if_nametoindex("en0");
    // setsockopt(fd, IPPROTO_IP, IP_BOUND_IF, &idx, sizeof(idx));

    // if(setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, ifname, 3) < 0) {
    //   perror("setsockopt() error");
    //   exit(2);
    // }
  //   char buffer[8192]; /* single packets are usually not bigger than 8192 bytes */
  //   while (1) {
  //     int len = recvfrom(fd , buffer , 8192 , 0 , NULL, NULL);

  //     // int len = read(fd, buffer, 8192);
  //     ipdump.dump(buffer, len);
  //     ipdump.dump("PACKET_END", 10);
  //     trace << len << "bytes received.";
  //   }

  } catch(const std::exception& e) {
    fatal << e.what();
  }

  return 1;
}
