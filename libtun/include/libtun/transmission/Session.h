#ifndef LIBTUN_TRANSMISSION_SESSION_INCLUDED
#define LIBTUN_TRANSMISSION_SESSION_INCLUDED

#include <string>
#include <vector>
#include <array>
#include <unordered_map>
#include <chrono>
#include <boost/asio/ip/address.hpp>
#include "./Command.h"
#include "./Cryptor.h"

namespace libtun {
namespace transmission {

  using boost::asio::ip::address;
  using std::chrono::steady_clock;

  class ClientSession {
  public:

    enum State: uint8_t {
      INIT,
      AUTH_SUCCESS,
      AUTH_FAILED,
      TRANSMITTING,
      TERMINATING,
      DESTROYED,
    };

    Cryptor cryptor;

    ClientSession(const address& ip, uint16_t port, uint16_t id) {
      _ip = ip;
      _port = port;
      _id = id;
    }

    State setState(State newState, TerminateReason reason = TerminateReason::EXIT) {
      if (
        newState == State::AUTH_SUCCESS &&
        (_state == State::INIT || _state == State::AUTH_FAILED)
      ) {
        _state = State::AUTH_SUCCESS;
      } else if (
        newState == State::AUTH_FAILED &&
        (_state == State::INIT || _state == State::AUTH_SUCCESS || _state == State::TRANSMITTING)
      ) {
        _state = State::AUTH_FAILED;
      } else if (newState == State::TRANSMITTING && _state == State::AUTH_SUCCESS) {
        _state = State::TRANSMITTING;
      } else if (
        newState == State::TERMINATING &&
        (_state != State::DESTROYED && _state == State::TERMINATING)
      ) {
        _state = State::TERMINATING;
        _reason = reason;
      } else if (newState == State::DESTROYED) {
        _state = State::DESTROYED;
      }

      return _state;
    }

    void updateLastActive() {
      _lastActive = steady_clock::now();
    }

  private:
    address _ip;
    uint16_t _port;
    uint16_t _id;
    State _state;
    TerminateReason _reason;

    steady_clock::time_point _lastActive;
    uint8_t _retry = 0;
  };

} // namespace transmission
} // namespace libtun

#endif