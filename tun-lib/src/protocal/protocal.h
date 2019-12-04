#ifndef TUN_LIB_PROTOCAL_INCLUDED
#define TUN_LIB_PROTOCAL_INCLUDED

#include <stdint.h>

namespace tunlib {

struct IPHeader {
  uint8_t versionAndHlen;
  uint8_t serviceType;
  uint16_t totalLen;
  uint16_t id;
  uint16_t flagAndOffset;
  uint32_t source;
  uint32_t destination;
  uint16_t options;
  uint16_t padding;
  uint8_t* data;
};

struct TCPHeader {

};

struct UDPHeader {

};

}

#endif