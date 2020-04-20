#ifndef LIBTUN_TRANSMISSION_RPC_PROTOCOL_INCLUDED
#define LIBTUN_TRANSMISSION_RPC_PROTOCOL_INCLUDED

#include <string>
#include <functional>
#include <tuple>
#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/system/error_code.hpp>
#include "../BufferPool.h"
#include "./constant.h"
#include "./Rpc.h"

namespace libtun {
namespace transmission {

  namespace endian = boost::endian;
  namespace boostErrc = boost::system::errc;
  using boost::asio::steady_timer;
  using boost::asio::io_context;
  using boost::asio::ip::udp;
  using boost::object_pool;
  using boost::system::error_code;

  /* A RPC PROTOCOL PACKET
  (REQUEST)
  ---------------------------------------------------------------------
  rpc packet...  |    type   |          len         | data (pairs)
  ---------------------------------------------------------------------

  (RESPONSE)
  ---------------------------------------------------------------------
  rpc packet...  |   error   |          len         | data (pairs)
  ---------------------------------------------------------------------
  */

  class RpcProtocol: public Rpc {
  public:

    // FUNCTION: (endpoint, username, password) => (error, key, iv)
    std::function<std::tuple<RpcErrorType, std::string, std::string>(udp::endpoint, std::string, std::string)> onConnect;

    // FUNCTION: (endpoint) => (error)
    std::function<RpcErrorType(udp::endpoint)> onPing;

    // FUNCTION: (endpoint) => (error)
    std::function<RpcErrorType(udp::endpoint)> onDisconnect;

    bool onRequest(Buffer buf, ControlBlock* control) {
      auto requestType = buf.data()[0];
      buf.moveFrontBoundary(1);

      if (requestType != RpcType::CONNECT && requestType != RpcType::DISCONNECT && requestType != RpcType::PING) {
        return false;
      }

      auto replyBuf = _bufferPool->alloc();
      replyBuf.moveFrontBoundary(10);
      replyBuf.size(1);

      if (requestType == RpcType::CONNECT) {
        std::string username = buf.readStringFromFront();
        std::string password = buf.readStringFromFront();
        RpcErrorType err;
        std::string key, iv;
        std::tie(err, key, iv) = onConnect(control->endpoint, username, password);
        replyBuf.data()[0] = err;
        buf.writeStringToBack(key);
        buf.writeStringToBack(iv);
      } else if (requestType == RpcType::DISCONNECT) {
        replyBuf.data()[0] = onDisconnect(control->endpoint);
      } else if (requestType == RpcType::PING) {
        replyBuf.data()[0] = onPing(control->endpoint);
      }

      control->buffer = replyBuf;
      control->onComplete = [_bufferPool = _bufferPool](error_code err, Buffer completeBuf, ControlBlock* control) {
        _bufferPool->free(completeBuf);
      };

      return true;
    }

    RpcProtocol(io_context* context, udp::socket* socket, Cryptor* cryptor, BufferPool<1600>* bufferPool):
      Rpc(context, socket, cryptor),
      _bufferPool(bufferPool) {

    }

    void connect(
      const udp::endpoint& to,
      const std::string& username,
      const std::string& password,
      std::function<void(RpcErrorType, std::string, std::string)> onConnectComplete
    ) {
      auto buf = _bufferPool->alloc();
      buf.moveFrontBoundary(10);
      buf.size(1);

      buf.data()[0] = RpcType::CONNECT;
      buf.writeStringToBack(username);
      buf.writeStringToBack(password);

      send(to, buf, [onConnectComplete, _bufferPool = _bufferPool](error_code err, Buffer replyBuf, ControlBlock* control) {
        if (err.failed()) {
          return onConnectComplete(RpcErrorType::NETWORK_ISSUE, "", "");
        }
        auto rpcError = replyBuf.data()[0];
        replyBuf.moveFrontBoundary(1);

        onConnectComplete((RpcErrorType)rpcError, replyBuf.readStringFromFront(), replyBuf.readStringFromFront());
        _bufferPool->free(control->buffer);
      });
    }

    void ping(const udp::endpoint& to, std::function<void(RpcErrorType)> onPingComplete) {
      auto buf = _bufferPool->alloc();
      buf.moveFrontBoundary(10);
      buf.size(1);
      buf.data()[0] = RpcType::PING;

      send(to, buf, [onPingComplete, _bufferPool = _bufferPool](error_code err, Buffer replyBuf, ControlBlock* control) {
        if (err.failed()) {
          return onPingComplete(RpcErrorType::NETWORK_ISSUE);
        }
        onPingComplete((RpcErrorType)(replyBuf.data()[0]));
        _bufferPool->free(control->buffer);
      });
    }

    void disconnect(const udp::endpoint& to, std::function<void(RpcErrorType)> onDisconnectComplete) {
      auto buf = _bufferPool->alloc();
      buf.moveFrontBoundary(10);
      buf.size(1);
      buf.data()[0] = RpcType::DISCONNECT;

      send(to, buf, [onDisconnectComplete, _bufferPool = _bufferPool](error_code err, Buffer replyBuf, ControlBlock* control) {
        if (err.failed()) {
          return onDisconnectComplete(RpcErrorType::NETWORK_ISSUE);
        }
        onDisconnectComplete((RpcErrorType)(replyBuf.data()[0]));
        _bufferPool->free(control->buffer);
      });
    }

  private:
    BufferPool<1600>* _bufferPool;

  };

} // namespace transmission
} // namespace libtun

#endif