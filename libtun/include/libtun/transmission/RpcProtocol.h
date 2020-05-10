#ifndef LIBTUN_TRANSMISSION_RPC_PROTOCOL_INCLUDED
#define LIBTUN_TRANSMISSION_RPC_PROTOCOL_INCLUDED

#include <string>
#include <functional>
#include <tuple>
#include <boost/asio.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/endian/conversion.hpp>
#include <boost/system/error_code.hpp>
#include <nlohmann/json.hpp>
#include "../logger.h"
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
  using nlohmann::json;

  /* A RPC PROTOCOL PACKET
  ---------------------------------------------
  rpc packet...  |          len         | json
  ---------------------------------------------

  json (request): { type: RpcType }
  json (response): { error: RpcErrorType, ...other_fields }
  */

  class RpcProtocol: public Rpc {
  public:

    // FUNCTION: (endpoint, username, password) => (error, key, iv)
    std::function<std::tuple<RpcErrorType, std::string, std::string>(udp::endpoint, std::string, std::string)> onConnect;

    // FUNCTION: (endpoint) => (error)
    std::function<RpcErrorType(udp::endpoint)> onPing;

    // FUNCTION: (endpoint) => (error)
    std::function<RpcErrorType(udp::endpoint)> onDisconnect;

    RpcProtocol(io_context* context, udp::socket* socket, Cryptor* cryptor, BufferPool<1600>* bufferPool, uint16_t retry = 10):
      Rpc(context, socket, cryptor, retry),
      _bufferPool(bufferPool) {
      onRequest = std::bind(&RpcProtocol::_requestHandler, this, std::placeholders::_1, std::placeholders::_2);
    }

    void sendJson(const udp::endpoint& to, const json& payload, std::function<void(json)> onReply) {
      auto buf = _bufferPool->alloc();
      buf.moveFrontBoundary(10);
      buf.size(0);
      buf.writeStringToBack(payload.dump());

      LOG_TRACE << fmt::format("RPC send: {}", payload.dump());

      send(to, buf, [&, onReply](error_code err, Buffer replyBuf, ControlBlock* control) {
        _bufferPool->free(control->buffer);

        if (err.failed()) {
          LOG_TRACE << fmt::format("RPC send failed: {}", err.message());
          return onReply({ { "error", RpcErrorType::NETWORK_ISSUE } });
        }

        auto replyStr = replyBuf.readStringFromFront();
        auto replyPayload = json::parse(replyStr, nullptr, false);

        LOG_TRACE << fmt::format("RPC send successful, reply: {} (len {})", replyStr, replyStr.size());

        if (!replyPayload.is_object() || !replyPayload["error"].is_number_integer()) {
          onReply({ { "error", RpcErrorType::NETWORK_ISSUE } });
        } else {
          onReply(replyPayload);
        }
      });
    }

    void connect(
      const udp::endpoint& to,
      const std::string& username,
      const std::string& password,
      std::function<void(RpcErrorType, std::string, std::string)> callback
    ) {
      json payload = {
        { "type", RpcType::CONNECT },
        { "username", username },
        { "password", password },
      };
      sendJson(to, payload, [callback](json replyPayload) {
        if (replyPayload["key"].is_string() && replyPayload["iv"].is_string()) {
          callback(replyPayload["error"], replyPayload["key"], replyPayload["iv"]);
        } else {
          callback(RpcErrorType::INVALID_INPUT, "", "");
        }
      });
    }

    void ping(const udp::endpoint& to, std::function<void(RpcErrorType)> callback) {
      json payload = { { "type", RpcType::PING } };
      sendJson(to, payload, [callback](json replyPayload) {
        callback(replyPayload["error"]);
      });
    }

    void disconnect(const udp::endpoint& to, std::function<void(RpcErrorType)> callback) {
      json payload = { { "type", RpcType::DISCONNECT } };
      sendJson(to, payload, [callback](json replyPayload) {
        callback(replyPayload["error"]);
      });
    }

  private:
    BufferPool<1600>* _bufferPool;

    bool _requestHandler(Buffer buf, ControlBlock* control) {
      auto jsonStr = buf.readStringFromFront();
      auto payload = json::parse(jsonStr, nullptr, false);

      LOG_TRACE << fmt::format("RPC new request: {} (len {})", jsonStr, jsonStr.size());

      if (!payload.is_object() || !payload["type"].is_number_integer()) {
        return false;
      }

      auto rpcType = payload["type"].get<uint8_t>();
      json replyPayload;

      if (rpcType == RpcType::CONNECT) {
        RpcErrorType err;
        std::string key, iv;
        std::tie(err, key, iv) = onConnect(control->endpoint, payload["username"].get<std::string>(), payload["password"].get<std::string>());
        replyPayload["error"] = err;
        replyPayload["key"] = key;
        replyPayload["iv"] = iv;
      } else if (rpcType == RpcType::DISCONNECT) {
        replyPayload["error"] = onDisconnect(control->endpoint);
      } else if (rpcType == RpcType::PING) {
        replyPayload["error"] = onPing(control->endpoint);
      } else {
        return false;
      }

      LOG_TRACE << fmt::format("RPC reply: {}", replyPayload.dump());

      auto replyBuf = _bufferPool->alloc();
      replyBuf.moveFrontBoundary(10);
      replyBuf.size(0);
      replyBuf.writeStringToBack(replyPayload.dump());

      control->buffer = replyBuf;
      control->onComplete = [&](error_code err, Buffer completeBuf, ControlBlock* control) {
        _bufferPool->free(completeBuf);
      };

      return true;
    }

  };

} // namespace transmission
} // namespace libtun

#endif