#pragma once

#include <string>
#include <map>
#include <set>
#include <functional>
#include <chrono>
#include <memory>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/system/error_code.hpp>
#include "../logger.h"
#include "../BufferPool.h"
#include "./constant.h"
#include "./Cryptor.h"

namespace libtun {
namespace transmission {

  namespace boostEc = boost::system::errc;
  using boost::system::error_code;
  using boost::signals2::signal;
  using boost::asio::steady_timer;
  using boost::asio::io_context;
  using boost::asio::ip::udp;
  using boost::asio::redirect_error;
  using boost::asio::awaitable;
  using boost::asio::co_spawn;
  using boost::asio::detached;
  using boost::asio::use_awaitable;

  /* A RPC PACKET
  ---------------------------------------------------
  |  Command  |          ID           |    CONTENT...
  ---------------------------------------------------
  */

  class Rpc {
  public:

    struct IDWithEndpoint {
      udp::endpoint endpoint;
      uint16_t id;

      bool operator == (const IDWithEndpoint& other) const {
        return id == other.id && endpoint == other.endpoint;
      }
      bool operator != (const IDWithEndpoint& other) const {
        return id != other.id || endpoint != other.endpoint;
      }
      bool operator < (const IDWithEndpoint& other) const {
        if (id != other.id) {
          return id < other.id;
        }
        return endpoint < other.endpoint;
      }
    };

    struct RequestControlBlock {
      RequestControlBlock(io_context* ctx): timer(*ctx) {}
      steady_timer timer;
      bool hasReplied = false;
      Buffer buffer;
    };

    Rpc(io_context* context, udp::socket* socket, Cryptor* cryptor, uint16_t retry = 10):
      _retry(retry),
      _context(context),
      _socket(socket),
      _cryptor(cryptor) {}

    void feed(const udp::endpoint& from, Buffer buf) {
      _cryptor->decrypt(buf.data() + 1, buf.size() - 1);
      uint8_t type = buf.readNumber<uint8_t>(0);
      uint16_t id = buf.readNumber<uint16_t>(1);
      buf.moveFrontBoundary(3);

      IDWithEndpoint idEp = {from, id};

      if (type == Command::REQUEST && _replying.find(idEp) == _replying.end()) {
        LOG_DEBUG << fmt::format("RAW_RPC request: #{} from: {} len {}", id, from.address().to_v4(), buf.size() + 3);
        onRequest(idEp, buf);
      } else if (type == Command::REPLY && _requesting.find(idEp) != _requesting.end()) {
        LOG_DEBUG << fmt::format("RAW_RPC reply: #{} from: {} len {}", id, from.address().to_v4(), buf.size() + 3);

        auto it = _requesting.find(idEp);
        if (!it->second->hasReplied) {
          it->second->buffer = buf;
          it->second->hasReplied = true;
          it->second->timer.cancel();
        }
      }
    }

  protected:

    signal<void(IDWithEndpoint, Buffer)> onRequest;

    awaitable<Buffer> send(const udp::endpoint& to, Buffer buf) {
      if (buf.prefixSpace() < 3) {
        throw boostEc::make_error_code(boostEc::no_buffer_space);
      }

      error_code ec;
      uint16_t id = _nextId();
      IDWithEndpoint idEp = {to, id};
      RequestControlBlock control(_context);
      _requesting[idEp] = &control;

      buf.moveFrontBoundary(-3);
      buf.writeNumber<uint8_t>(Command::REQUEST, 0);
      buf.writeNumber<uint16_t>(id, 1);
      _cryptor->encrypt(buf.data() + 1, buf.size() - 1);

      for (uint16_t i = 0; i < _retry && !control.hasReplied; i++) {
        co_await _socket->async_send_to(buf.toConstBuffer(), to, redirect_error(use_awaitable, ec));
        control.timer.expires_after(std::chrono::seconds(1));
        co_await control.timer.async_wait(redirect_error(use_awaitable, ec));
      }

      _requesting.erase(idEp);

      if (control.hasReplied) {
        co_return control.buffer;
      }
      if (ec.failed()) {
        throw ec;
      }
      throw boostEc::make_error_code(boostEc::timed_out);
    }

    awaitable<void> reply(IDWithEndpoint idEp, Buffer buf) {
      if (buf.prefixSpace() < 3) {
        throw boostEc::make_error_code(boostEc::no_buffer_space);
      }

      buf.moveFrontBoundary(-3);
      buf.writeNumber<uint8_t>(Command::REPLY, 0);
      buf.writeNumber<uint16_t>(idEp.id, 1);
      _cryptor->encrypt(buf.data() + 1, buf.size() - 1);

      error_code ec;
      steady_timer timer(*_context);
      _replying.insert(idEp);

      for (uint16_t i = 0; i < _retry; i++) {
        co_await _socket->async_send_to(buf.toConstBuffer(), idEp.endpoint, redirect_error(use_awaitable, ec));
        timer.expires_after(std::chrono::seconds(1));
        co_await timer.async_wait(redirect_error(use_awaitable, ec));
      }

      _requesting.erase(idEp);

      if (ec.failed()) {
        throw ec;
      }
    }

  private:
    uint16_t _id = 1;
    uint16_t _retry;
    io_context* _context;
    udp::socket* _socket;
    Cryptor* _cryptor;
    std::set<IDWithEndpoint> _replying;
    std::map<IDWithEndpoint, RequestControlBlock*> _requesting;

    uint16_t _nextId() {
      if (_id == 0xffff) {
        _id = 1;
        return 0xffff;
      }
      return _id++;
    }

  };

} // namespace transmission
} // namespace libtun
