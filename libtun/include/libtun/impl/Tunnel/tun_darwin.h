#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>
#include <sys/uio.h>
#include <net/if_utun.h>
#include <stdint.h>
#include <fmt/core.h>
#include <boost/asio/buffer.hpp>

#include <libtun/Exception.h>
#include <libtun/Tunnel.h>

namespace libtun {

  using boost::asio::const_buffer;
  using boost::asio::mutable_buffer;

  void Tunnel::_openTun() {
    fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if (fd == -1) {
      throw Exception(fmt::format("tun socket open failed: {}", strerror(errno)));
    }

    struct ctl_info ctlInfo = {
      .ctl_id = 0,
      .ctl_name = UTUN_CONTROL_NAME,
    };
    if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1) {
      throw Exception(fmt::format("tun ioctl error: {}", strerror(errno)));
    }

    struct sockaddr_ctl sc = {
      .sc_len = sizeof(sc),
      .sc_family = AF_SYSTEM,
      .ss_sysaddr = AF_SYS_CONTROL,
      .sc_id = ctlInfo.ctl_id,
      .sc_unit = 0,
    };

    if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) == -1) {
      throw Exception(fmt::format("tun connect error: {}", strerror(errno)));
    }

    char tunname[20];
    socklen_t tunnameLen = sizeof(tunname);
    if (getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, tunname, &tunnameLen)) {
      throw Exception(fmt::format("tun get name error: {}", strerror(errno)));
    }

    ifName = tunname;
  }

  inline ssize_t _read(int fd, void* buf, size_t len) {
    struct iovec bufs[] = {
      { .iov_base = buf, .iov_len = 4 },
      { .iov_base = buf, .iov_len = len },
    };
    return readv(fd, bufs, 2);
  }

  mutable_buffer Tunnel::read(mutable_buffer buffer) {
    auto len = _read(fd, buffer.data(), buffer.size());
    return mutable_buffer(buffer.data(), len < 4 ? 0 : (len - 4));
  }

  uint16_t Tunnel::read() {
    auto len = _read(fd, readBuffer, MTU);
    return len < 4 ? 0 : static_cast<uint16_t>(len - 4);
  }

  const uint8_t ip4Flag[] = {0, 0, 0, 4};

  uint16_t Tunnel::write(const const_buffer& buffer) {
    struct iovec bufs[] = {
      { .iov_base = (void*)ip4Flag, .iov_len = 4 },
      { .iov_base = (void*)buffer.data(), .iov_len = buffer.size() },
    };
    auto len = writev(fd, bufs, 2);
    return len < 4 ? 0 : static_cast<uint16_t>(len - 4);
  }

}
