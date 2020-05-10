#ifndef LIBTUN_TRANSMISSION_CONSTANT_INCLUDED
#define LIBTUN_TRANSMISSION_CONSTANT_INCLUDED

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
  };

  enum RpcErrorType: uint8_t {
    SUCCESS,
    INVALID_INPUT,
    WRONG_CREDENTIAL,
    TOO_MANY_CONNECTION,
    NOT_CONNECTED,
    NETWORK_ISSUE,
  };

  enum SessionStatus: uint8_t {
    IDLE,
    CONNECTED,
    DISCONNECTED,
    ERROR,
  };

} // namespace transmission
} // namespace libtun

#endif