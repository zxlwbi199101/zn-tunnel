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
#include "./RpcBuffer.h"
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
  ---------------------------------------------------------
  |command |       id       |       len      | json string
  ---------------------------------------------------------
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
      json data;
    };

    signal<void(IDWithEndpoint, json)> onRequest;

    Rpc(io_context* context, udp::socket* socket, Cryptor* cryptor, BufferPool<1600>* pool, uint16_t retry = 10):
      _retry(retry),
      _context(context),
      _socket(socket),
      _cryptor(cryptor),
      _pool(pool) {}

    void feed(const udp::endpoint& from, Buffer incomingBuf) {
      RpcBuffer buf(incomingBuf, _cryptor);
      IDWithEndpoint idEp = {from, buf.id()};

      if (!buf.isValid()) {
        return;
      }

      if (buf.command() == Command::REQUEST && _replying.find(idEp) == _replying.end()) {
        LOG_DEBUG << fmt::format("RAW_RPC request: #{} from: {} len {}", idEp.id, from.address().to_v4(), buf.size());
        onRequest(idEp, buf.content());
      } else if (buf.command() == Command::REPLY) {
        auto it = _requesting.find(idEp);

        if (it != _requesting.end() && !it->second->hasReplied) {
          LOG_DEBUG << fmt::format("RAW_RPC reply: #{} from: {} len {}", idEp.id, from.address().to_v4(), buf.size());
          it->second->data = buf.content();
          it->second->hasReplied = true;
          it->second->timer.cancel();
        }
      }
    }

    awaitable<json> send(const udp::endpoint& to, json data) {
      error_code ec;
      IDWithEndpoint idEp = {to, _nextId()};
      RequestControlBlock control(_context);
      _requesting[idEp] = &control;

      RpcBuffer buf(_pool->alloc(), _cryptor, false);
      buf.command(Command::REQUEST).id(idEp.id).content(data);

      for (uint16_t i = 0; i < _retry && !control.hasReplied; i++) {
        co_await _socket->async_send_to(buf.toConstBuffer(), to, redirect_error(use_awaitable, ec));
        control.timer.expires_after(std::chrono::seconds(1));
        co_await control.timer.async_wait(redirect_error(use_awaitable, ec));
      }

      _requesting.erase(idEp);
      _pool->free(buf);

      if (!control.hasReplied) {
        co_return { { "error", RpcErrorType::TIMEOUT } };
      }
      co_return control.data;
    }

    awaitable<void> reply(IDWithEndpoint idEp, json data) {
      error_code ec;
      steady_timer timer(*_context);
      _replying.insert(idEp);

      RpcBuffer buf(_pool->alloc(), _cryptor, false);
      buf.command(Command::REPLY).id(idEp.id).content(data);

      for (uint16_t i = 0; i < _retry; i++) {
        co_await _socket->async_send_to(buf.toConstBuffer(), idEp.endpoint, redirect_error(use_awaitable, ec));
        timer.expires_after(std::chrono::seconds(1));
        co_await timer.async_wait(redirect_error(use_awaitable, ec));
      }

      _requesting.erase(idEp);
      _pool->free(buf);
    }

  private:
    uint16_t _id = 1;
    uint16_t _retry;
    io_context* _context;
    udp::socket* _socket;
    Cryptor* _cryptor;
    BufferPool<1600>* _pool;
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
