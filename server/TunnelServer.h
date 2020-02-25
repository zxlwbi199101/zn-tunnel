#ifndef SERVER_TUNNEL_SERVER_INCLUDED
#define SERVER_TUNNEL_SERVER_INCLUDED

#include <exception>
#include <memory>
#include <string>
#include <array>
#include <boost/asio.hpp>
#include <libtun/transmission.h>
#include <libtun/napt.h>
#include <libtun/BufferPool.h>

namespace znserver {

  using boost::asio::ip::udp;
  using boost::asio::io_context;
  using boost::asio::ip::address_v4;
  using libtun::transmission::ClientSession;
  using libtun::transmission::Communicator;
  using libtun::transmission::Command;
  using libtun::NAPT;
  using libtun::BufferPool;

  struct TunnelServerConfig {
    uint16_t listenPort;
    uint16_t portFrom;
    uint16_t portTo;
  };

  class TunnelServer {
  public:
    typedef std::shared_ptr<ClientSession> SessionPtr;

    std::array<SessionPtr, 255> clients = {nullptr};
    NAPT<address_v4, address_v4> tcpNapt;
    NAPT<address_v4, address_v4> udpNapt;

    TunnelServer(io_context* context, TunnelServerConfig config, BufferPool<2000>* pool):
      _contextPtr(context),
      _poolPtr(pool),
      _socket(context, udp::endpoint(udp::v4(), config.listenPort)),
      tcpNapt(config.portFrom, config.portTo),
      udpNapt(config.portFrom, config.portTo) {}

  private:
    io_context* _contextPtr;
    BufferPool<2000>* _poolPtr;
    udp::socket _socket;

    void _receive() {

    }

    uint8_t _newClient() {
      for (int i = 0; i < clients.size(); i++) {
        if (clients[i] == nullptr) return i;
      }
      return -1;
    }

  };

} // namespace znserver

#endif