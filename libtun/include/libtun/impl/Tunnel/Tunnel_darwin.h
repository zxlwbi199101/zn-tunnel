#pragma once

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>
#include <sys/uio.h>
#include <net/if_utun.h>

#include <stdint.h>
#include <fmt/core.h>
#include <libtun/Exception.h>
#include <libtun/BufferPool.h>

namespace libtun {
namespace impl {

  class TunnelImpl {
  public:
    int fd = -1;
    std::string ifName;

    void openTunnel() {
      fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
      if (fd == -1) {
        throw Exception(fmt::format("tunnel socket open failed: {}", strerror(errno)));
      }

      ctl_info ctlInfo = {
        .ctl_id = 0,
        .ctl_name = UTUN_CONTROL_NAME,
      };
      if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1) {
        throw Exception(fmt::format("tunnel ioctl error: {}", strerror(errno)));
      }

      sockaddr_ctl sc = {
        .sc_len = sizeof(sc),
        .sc_family = AF_SYSTEM,
        .ss_sysaddr = AF_SYS_CONTROL,
        .sc_id = ctlInfo.ctl_id,
        .sc_unit = 0,
      };

      if (connect(fd, (sockaddr *)&sc, sizeof(sc)) == -1) {
        throw Exception(fmt::format("tunnel connect error: {}", strerror(errno)));
      }

      char tunname[20];
      socklen_t tunnameLen = sizeof(tunname);
      if (getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, tunname, &tunnameLen)) {
        throw Exception(fmt::format("get tunnel name error: {}", strerror(errno)));
      }

      ifName = tunname;
    }

    void close() {
      if (fd > 0) {
        ::close(fd);
        fd = -1;
      }
    }

    Buffer read(Buffer buf) {
      if (fd <= 0) return buf;

      auto len = ::read(fd, buf.data(), buf.size());
      buf.size(len);
      buf.moveFrontBoundary(4);
      return buf;
    }

    int write(Buffer buf) {
      if (buf.prefixSpace() < 4 || fd <= 0) {
        return 0;
      }
      buf.moveFrontBoundary(-4);
      auto data = buf.data();
      *((uint32_t*)data[0]) = 4;

      return ::write(fd, data, buf.size());
    }

  };

} // namespace impl
} // namespace libtun
