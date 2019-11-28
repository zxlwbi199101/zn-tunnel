#include "./logger.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/sys_domain.h>
#include <sys/kern_control.h>
#include <net/if_utun.h>

int tun(void) {
  int fd = socket(PF_SYSTEM, SOCK_DGRAM, SYSPROTO_CONTROL);
  if (fd == -1) {
    fatal << "socket open failed: " << strerror(errno);
    return -1;
  }

  struct ctl_info ctlInfo = {
    .ctl_id = 0,
    .ctl_name = UTUN_CONTROL_NAME
  };
  if (ioctl(fd, CTLIOCGINFO, &ctlInfo) == -1) {
    fatal << "ioctl error: " << strerror(errno);
    close(fd);
    return -1;
  }

  struct sockaddr_ctl sc = {
    .sc_len = sizeof(sc),
    .sc_family = AF_SYSTEM,
    .ss_sysaddr = AF_SYS_CONTROL,
    .sc_id = ctlInfo.ctl_id,
    .sc_unit = 0
  };

  if (connect(fd, (struct sockaddr *)&sc, sizeof(sc)) == -1) {
    fatal << "connect error: " << strerror(errno);
    close(fd);
    return -1;
  }
  return fd;
}

int main(int argc, char **argv) {
  enableFileLog("/Users/xzhao048/Desktop/zntun%N.log");
  enableConsoleLog();

  int utunfd = tun();

  if (utunfd == -1) {
    fprintf(stderr, "Unable to establish UTUN descriptor - aborting\n");
    exit(1);
  }

  fprintf(stderr, "Utun interface is up.. Configure IPv4 using \"ifconfig utun1 _ipA_ _ipB_\"\n");
  fprintf(stderr, "                       Configure IPv6 using \"ifconfig utun1 inet6 _ip6_\"\n");
  fprintf(stderr, "Then (e.g.) ping _ipB_ (IPv6 will automatically generate ND messages)\n");

  // PoC - Just dump the packets...
  uint8_t buf[1500];

  while (1) {
    uint16_t len = read(utunfd, buf, 1500);

    dumpIpPacket(buf, len);
    dumpIpPacket((uint8_t*)"IP_PACKET_END", 13);

    // for (ssize_t i = 0; i < len; i++) {
    //    printf("%4d", buf[i]);
    //    if (i % 4 == 3) printf("\n");
    // }
    // printf("\n");
  }

  return 0;
}
