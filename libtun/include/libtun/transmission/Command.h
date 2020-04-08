#ifndef LIBTUN_TRANSMISSION_COMMAND_INCLUDED
#define LIBTUN_TRANSMISSION_COMMAND_INCLUDED

#include <boost/asio/ip/address_v4.hpp>
#include <string>

namespace libtun {
namespace transmission {

  enum Command: uint8_t {
    REQUEST,
    REPLY,
    TRANSMIT,
  };

  enum RpcType: uint8_t {
    CONNECT,
    PING,
    DISCONNECT,
    ERROR,
  };

  enum RpcErrorType: uint8_t {
    SUCCESS,
    INVALID_INPUT,
    WRONG_CREDENTIAL,
    TOO_MANY_CONNECTION,
    NOT_CONNECTED,
  };

} // namespace transmission
} // namespace libtun

#endif