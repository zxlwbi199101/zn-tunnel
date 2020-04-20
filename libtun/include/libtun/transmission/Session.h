#ifndef LIBTUN_TRANSMISSION_SESSION_INCLUDED
#define LIBTUN_TRANSMISSION_SESSION_INCLUDED

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <chrono>
#include <boost/asio/ip/udp.hpp>
#include "./constant.h"
#include "./Cryptor.h"

namespace libtun {
namespace transmission {

  using boost::asio::ip::udp;
  using std::chrono::system_clock;

  class Session {
  public:

    Cryptor cryptor;
    udp::endpoint endpoint;
    uint16_t clientId;
    system_clock::time_point lastActiveAt = system_clock::now();
    uint64_t transmittedBytes = 0;
    SessionStatus status = SessionStatus::IDLE;
    RpcErrorType error;

    Session(udp::endpoint ep, uint16_t id): endpoint(ep), clientId(id) {}
    Session(uint16_t id): clientId(id) {}
    Session() {}

    bool isConnect() {
      return status == SessionStatus::CONNECTED;
    }

    void updateTransmit(int len) {
      lastActiveAt = system_clock::now();
      transmittedBytes += len;
    }

  };

} // namespace transmission
} // namespace libtun

#endif