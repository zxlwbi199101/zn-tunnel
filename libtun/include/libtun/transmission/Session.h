#pragma once

#include <chrono>
#include <boost/asio/ip/udp.hpp>
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

    Session() {}

    bool isConnected() {
      return status == SessionStatus::CONNECTED;
    }

    bool isIdle() {
      return status == SessionStatus::IDLE;
    }

    void reset(uint16_t id, udp::endpoint from) {
      clientId = id;
      cryptor = Cryptor();
      endpoint = from;
      lastActiveAt = system_clock::now();
      status = SessionStatus::CONNECTED;
      transmittedBytes = 0;
      error = RpcErrorType::SUCCESS;
    }

    void updateTransmit(int len) {
      lastActiveAt = system_clock::now();
      transmittedBytes += len;
    }

  };

} // namespace transmission
} // namespace libtun
