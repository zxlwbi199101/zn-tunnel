#pragma once

#include <exception>
#include <string>
#include <vector>
#include <boost/asio.hpp>
#include <boost/endian.hpp>
#include <libtun/transmission.h>
#include <libtun/napt.h>
#include <libtun/protocol.h>
#include <libtun/BufferPool.h>
#include <libtun/RawSocket.h>

namespace znserver {

  namespace endian = boost::endian;
  using boost::asio::ip::udp;
  using boost::asio::io_context;
  using boost::asio::ip::address_v4;
  using libtun::NAPT;
  using libtun::BufferPool;
  using libtun::RawSocket;
  using namespace libtun::protocol;
  using namespace libtun::transmission;

  struct TunnelServerConfig {
    uint16_t listenPort;
    uint16_t portFrom;
    uint16_t portTo;
    uint8_t maxSessions;
    std::string key;
    std::string iv;
  };

  class TunnelServer {
  public:

    TunnelServerConfig serverConfig;
    NAPT<address_v4, address_v4> tcpNapt;
    NAPT<address_v4, address_v4> udpNapt;
    std::vector<Session> sessions;

    TunnelServer(TunnelServerConfig config, BufferPool<1600>* pool):
      tcpNapt(config.portFrom, config.portTo),
      udpNapt(config.portFrom, config.portTo),
      sessions(config.maxSessions),
      serverConfig(config),
      _bufferPool(pool),
      _socket(_context, udp::endpoint(udp::v4(), config.listenPort)),
      _cryptor(config.key, config.iv),
      _rpc(&_context, &_socket, &_cryptor, pool) {}

    void start();
    void stop();

  private:
    io_context _context;
    BufferPool<1600>* _bufferPool;
    udp::socket _socket;
    udp::endpoint _receiveEp;
    RpcProtocol _rpc;
    Cryptor _cryptor;
    RawSocket _rawSocket;

    void _onSocketReceive(error_code err, std::size_t transfered, libtun::Buffer buf);
    void _processTransmit(const libtun::Buffer& buf);
    void _removeSession(uint16_t id);

    std::tuple<RpcErrorType, std::string, std::string> _rpcConnectHandler(udp::endpoint from, std::string name, std::string password);
    RpcErrorType _rpcPingHandler(udp::endpoint from);
    RpcErrorType _rpcDisconnectHandler(udp::endpoint from);

    void _rawSocketLoopHandler();
    void _rawSocketPacketHandler(uint8_t* data, uint32_t size);
  };

} // namespace znserver
