#ifndef TUN_LIB_TUN_INCLUDED
#define TUN_LIB_TUN_INCLUDED

#include <stdint.h>

namespace tunlib {

  const uint32_t MTU = 1500;

  struct TunBuffer {
    uint16_t len;
    uint8_t *data;
  };

  class Tun {
  public:
    int fd = -1;
    std::string ifName;

    bool opening() { return fd > 0; }
    bool open();
    void close();
    TunBuffer read();
    uint16_t write(const uint8_t* buf, uint16_t len);

  private:
    uint8_t readBuf[MTU + 4];
  };
}

#endif
