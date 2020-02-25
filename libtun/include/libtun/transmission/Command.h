#ifndef LIBTUN_TRANSMISSION_COMMAND_INCLUDED
#define LIBTUN_TRANSMISSION_COMMAND_INCLUDED

#include <boost/asio/ip/address_v4.hpp>
#include <string>

namespace libtun {
namespace transmission {

  enum Command: uint8_t {
    // for client
    REQ_AUTH,

    // for server
    RES_AUTH_SECCESS,

    // for both
    TRANSMIT,
    TERMINATE,
    PING,
    PONG,
  };

  enum TerminateReason: uint8_t {
    EXIT,
    NOT_AVAILABLE,
    AUTH_FAILED,
  };

} // namespace transmission
} // namespace libtun

#endif