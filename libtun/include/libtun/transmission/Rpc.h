#ifndef LIBTUN_TRANSMISSION_RPC_INCLUDED
#define LIBTUN_TRANSMISSION_RPC_INCLUDED

#include <string>
#include <map>
#include <functional>
#include <chrono>
#include <boost/asio.hpp>
#include <boost/container_hash/hash.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/system/error_code.hpp>
#include "../BufferPool.h"
#include "./constant.h"
#include "./Cryptor.h"

namespace libtun {
namespace transmission {

  namespace endian = boost::endian;
  namespace boostErrc = boost::system::errc;
  using boost::asio::steady_timer;
  using boost::asio::io_context;
  using boost::asio::ip::udp;
  using boost::object_pool;
  using boost::system::error_code;

  /* A RPC PACKET
  ---------------------------------------------------
  |  Command  |          ID           |    CONTENT...
  ---------------------------------------------------
  */

  class Rpc {
  public:

    struct ControlBlock {
      int8_t remain = 0;
      steady_timer timer;
      udp::endpoint endpoint;
      uint16_t id;
      Buffer buffer;
      std::function<void(error_code, Buffer, ControlBlock*)> onComplete;
    };

    std::function<bool(Buffer, ControlBlock*)> onRequest;

    Rpc(io_context* context, udp::socket* socket, Cryptor* cryptor):
      _context(context),
      _socket(socket),
      _cryptor(cryptor) {}

    void feed(const udp::endpoint& from, Buffer buf) {
      auto data = buf.data();
      _cryptor->decrypt(data + 1, buf.size() - 1);
      uint8_t type = data[0];
      uint16_t id = endian::big_to_native(*((uint16_t*)(data + 1)));
      buf.moveFrontBoundary(3);

      if (type == Command::REQUEST && _replying.find({from, id}) == _replying.end()) {
        ControlBlock* control = _pool.malloc();
        control->remain = 10;
        control->timer = steady_timer(*_context);
        control->endpoint = from;
        control->id = id;
        control->buffer = buf;
        control->onComplete = nullptr;

        if (onRequest(buf, control)) {
          if (control->buffer.prefixSpace() < 3) {
            control->onComplete(boostErrc::make_error_code(boostErrc::no_buffer_space), Buffer(), nullptr);
            _pool.free(control);
            return;
          }
          control->buffer.moveFrontBoundary(-3);

          auto inputData = control->buffer.data();
          inputData[0] = Command::REPLY;
          *((uint16_t*)(inputData + 1)) = endian::native_to_big(control->id);
          _cryptor->encrypt(inputData + 1, control->buffer.size() - 1);

          _replying[{from, id}] = control;
          _onReplyTimer(error_code(), control);
        } else {
          _pool.free(control);
        }
      } else if (type == Command::REPLY && _requesting.find(id) != _requesting.end()) {
        auto it = _requesting.find(id);
        it->second->onComplete(error_code(), buf, it->second);
        it->second->remain = 0;
      }
    }

    void send(
      const udp::endpoint& to,
      Buffer buf,
      std::function<void(error_code, Buffer, ControlBlock*)> onComplete
    ) {
      if (buf.prefixSpace() < 3) {
        onComplete(boostErrc::make_error_code(boostErrc::no_buffer_space), Buffer(), nullptr);
        return;
      }

      uint16_t id = _nextId();

      buf.moveFrontBoundary(-3);
      auto data = buf.data();
      data[0] = Command::REQUEST;
      *((uint16_t*)(data + 1)) = endian::native_to_big(id);
      _cryptor->encrypt(data + 1, buf.size() - 1);

      ControlBlock* control = _pool.malloc();
      control->remain = 10;
      control->timer = steady_timer(*_context);
      control->endpoint = to;
      control->id = id;
      control->buffer = buf;
      control->onComplete = onComplete;

      _requesting[id] = control;
      _onRequestTimer(error_code(), control);
    }

  private:

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

    uint16_t _id = 1;
    io_context* _context;
    udp::socket* _socket;
    Cryptor* _cryptor;

    object_pool<ControlBlock> _pool;
    std::map<IDWithEndpoint, ControlBlock*> _replying;
    std::map<uint16_t, ControlBlock*> _requesting;

    uint16_t _nextId() {
      if (_id == 0xffff) {
        _id = 1;
        return 0xffff;
      }
      return _id++;
    }

    void _onReplyTimer(const error_code& err, ControlBlock* control) {
      if (--control->remain <= 0 || err.failed()) {
        control->onComplete(err, control->buffer, control);
        _replying.erase({control->endpoint, control->id});
        _pool.free(control);
        return;
      }

      _socket->async_send_to(
        control->buffer.toConstBuffer(),
        control->endpoint,
        [control, this](const error_code& sendErr, std::size_t transfered) {
          if (sendErr.failed()) {
            _onReplyTimer(sendErr, control);
          } else {
            control->timer.expires_after(std::chrono::seconds(1));
            control->timer.async_wait(std::bind(&Rpc::_onReplyTimer, this, std::placeholders::_1, control));
          }
        }
      );
    }

    void _onRequestTimer(const error_code& err, ControlBlock* control) {
      if (--control->remain <= 0 || err.failed()) {
        if (control->remain == 0) {
          control->onComplete(boostErrc::make_error_code(boostErrc::timed_out), Buffer(), control);
        } else if (err.failed()) {
          control->onComplete(err, Buffer(), control);
        }
        _requesting.erase(control->id);
        _pool.free(control);
        return;
      }

      _socket->async_send_to(
        control->buffer.toConstBuffer(),
        control->endpoint,
        [control, this](const error_code& sendErr, std::size_t transfered) {
          if (sendErr.failed()) {
            _onRequestTimer(sendErr, control);
          } else {
            control->timer.expires_after(std::chrono::seconds(1));
            control->timer.async_wait(std::bind(&Rpc::_onRequestTimer, this, std::placeholders::_1, control));
          }
        }
      );
    }

  };

} // namespace transmission
} // namespace libtun

#endif