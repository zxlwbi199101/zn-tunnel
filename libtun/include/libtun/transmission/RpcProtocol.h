#pragma once

#include <string>
#include <tuple>
#include <boost/asio.hpp>
#include <boost/signals2.hpp>
#include <boost/system/error_code.hpp>
#include <nlohmann/json.hpp>
#include "../logger.h"
#include "../BufferPool.h"
#include "./constant.h"
#include "./Rpc.h"

namespace libtun {
namespace transmission {

  using boost::signals2::signal;
  using boost::asio::steady_timer;
  using boost::asio::io_context;
  using boost::asio::ip::udp;
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

    // FUNCTION: (endpoint, username, password)
    signal<void(IDWithEndpoint, std::string, std::string)> onConnect;

    // FUNCTION: (endpoint)
    signal<void(IDWithEndpoint)> onPing;

    // FUNCTION: (endpoint)
    signal<void(IDWithEndpoint)> onDisconnect;

    RpcProtocol(io_context* context, udp::socket* socket, Cryptor* cryptor, BufferPool<1600>* bufferPool, uint16_t retry = 10):
      Rpc(context, socket, cryptor, retry),
      _bufferPool(bufferPool) {
      onRequest.connect(std::bind(&RpcProtocol::_requestHandler, this, std::placeholders::_1, std::placeholders::_2));
    }

    awaitable<json> sendJson(const udp::endpoint& to, const json& payload) {
      auto buf = _bufferPool->alloc();
      buf.moveFrontBoundary(10);
      buf.size(0);
      buf.writeStringToBack(payload.dump());

      json replayPayload = { { "error", RpcErrorType::NETWORK_ISSUE } };

      try {
        auto replyBuf = co_await send(to, buf);
        auto replyStr = replyBuf.readStringFromFront();
        replyPayload = json::parse(replyStr, nullptr, false);
        LOG_TRACE << fmt::format("RPC send successful, request: {}, reply({}): {}", payload.dump(), replyStr.size(), replyStr);
      } catch(std::exception ex) {
        LOG_ERROR << fmt::format("RPC send failed, request: {}, error: {}", payload.dump(), ex.what());
      }

      _bufferPool->free(buf);
      co_return replyPayload;
    }

    awaitable<void> replyJson(IDWithEndpoint& idEp, const json& payload) {
      auto buf = _bufferPool->alloc();
      buf.moveFrontBoundary(10);
      buf.size(0);
      buf.writeStringToBack(payload.dump());

      try {
        co_await reply(idEp, buf);
        LOG_TRACE << fmt::format("RPC reply successful, payload: {}", payload.dump());
      } catch (std::exception& err) {
        LOG_ERROR << fmt::format("RPC reply failed, payload: {}", payload.dump());
      }

      _bufferPool->free(buf);
    }

    awaitable<std::tuple<RpcErrorType, std::string, std::string>> connect(
      const udp::endpoint& to,
      const std::string& username,
      const std::string& password
    ) {
      json reply = co_await sendJson(to, {
        { "type", RpcType::CONNECT },
        { "username", username },
        { "password", password },
      });

      if (reply["error"].is_number_integer() && reply["error"] != RpcErrorType::SUCCESS) {
        co_return std::make_tuple(reply["error"], "", "");
      }
      if (reply["key"].is_string() && reply["iv"].is_string()) {
        co_return std::make_tuple(reply["error"], reply["key"], reply["iv"]);
      }
      co_return std::make_tuple(RpcErrorType::INVALID_INPUT, "", "");
    }

    awaitable<RpcErrorType> ping(const udp::endpoint& to) {
      json reply = co_await sendJson(to, { { "type", RpcType::PING } });
      co_return reply["error"];
    }

    awaitable<RpcErrorType> disconnect(const udp::endpoint& to) {
      json reply = co_await sendJson(to, { { "type", RpcType::DISCONNECT } });
      co_return reply["error"];
    }

  private:
    BufferPool<1600>* _bufferPool;

    void _requestHandler(IDWithEndpoint idEp, Buffer buf) {
      auto jsonStr = buf.readStringFromFront();
      auto payload = json::parse(jsonStr, nullptr, false);

      LOG_TRACE << fmt::format("RPC new request: {} (len {})", jsonStr, jsonStr.size());

      if (!payload.is_object() || !payload["type"].is_number_integer()) {
        return;
      }

      auto rpcType = payload["type"].get<uint8_t>();

      if (rpcType == RpcType::CONNECT) {
        onConnect(idEp, payload["username"].get<std::string>(), payload["password"].get<std::string>());
      } else if (rpcType == RpcType::DISCONNECT) {
        onDisconnect(idEp);
      } else if (rpcType == RpcType::PING) {
        onPing(idEp);
      }
    }

  };

} // namespace transmission
} // namespace libtun
