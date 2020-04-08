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
#include "./Command.h"
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
  -----------------------------------------------------------
  rpc packet...  |    type   |   error   |          len         | data
  -----------------------------------------------------------
  */

  class RpcProtocol: Rpc {
  public:

    // FUNCTION: (username, password) => (error, key, iv)
    std::function<std::tuple<RpcErrorType, std::string, std::string>(std::string, std::string)> onConnect;

    // FUNCTION: () => (error)
    std::function<RpcErrorType()> onPing;

    // FUNCTION: () => (error)
    std::function<RpcErrorType()> onDisconnect;

    bool onRequest(Buffer buf, ControlBlock* control) {
      auto data = buf.data();
      auto replyBUf = _pool
      if (data[0] == RpcType::CONNECT) {

      } else if (data[0] == RpcType::DISCONNECT) {

      } else if (data[0] == RpcType::PING) {

      }
      return false;
    }
    // std::function<bool(Buffer, ControlBlock*)> onRequest;

    RpcProtocol(io_context* context, udp::socket* socket, Cryptor* cryptor, BufferPool<1600>* bufferPool):
      Rpc(context, socket, cryptor),
      _bufferPool(pool) {

    }

    void connect(const std::string& name, const std::string& password) {

    }

  private:
    BufferPool<1600>* _pool;
  };

} // namespace transmission
} // namespace libtun

#endif