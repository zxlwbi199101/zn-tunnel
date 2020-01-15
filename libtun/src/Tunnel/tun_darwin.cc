#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>
#include <net/if_utun.h>
#include <stdint.h>
#include <fmt/core.h>

#include <libtun/logger.h>
#include <libtun/Exception.h>
#include <libtun/Tunnel.h>

namespace libtun {

  int openTun(std::string& name) {
    int fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
    if (fd == -1) {
      throw Exception(fmt::format("tun socket open failed: {}", strerror(errno)));
    }

    struct ctl_info ctlInfo = {
      .ctl_id = 0,
      .ctl_name = UTUN_CONTROL_NAME
    };
    if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1) {
      throw Exception(fmt::format("tun ioctl error: {}", strerror(errno)));
    }

    struct sockaddr_ctl sc = {
      .sc_len = sizeof(sc),
      .sc_family = AF_SYSTEM,
      .ss_sysaddr = AF_SYS_CONTROL,
      .sc_id = ctlInfo.ctl_id,
      .sc_unit = 0
    };

    if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) == -1) {
      throw Exception(fmt::format("tun connect error: {}", strerror(errno)));
    }

    char tunname[20];
    socklen_t tunnameLen = sizeof(tunname);
    if (getsockopt(fd, SYSPROTO_CONTROL, UTUN_OPT_IFNAME, tunname, &tunnameLen)) {
      throw Exception(fmt::format("tun get name error: {}", strerror(errno)));
    }

    name = tunname;
    return fd;
  }

  TunBuffer readTun(int fd, uint8_t* buf, uint16_t len) {
    return {
      .len = static_cast<uint16_t>(read(fd, buf, len) - 4),
      .data = buf + 4
    };
  }

  const uint8_t ip4Flag[] = {0, 0, 0, 4};

  uint16_t writeTun(int fd, const uint8_t* buf, uint16_t len) {
    write(fd, ip4Flag, 4);
    return write(fd, buf, len);
  }

}
