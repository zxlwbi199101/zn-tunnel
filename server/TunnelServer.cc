#include <fmt/core.h>
#include <libtun/logger.h>
#include "./TunnelServer.h"

namespace znserver {

  void TunnelServer::start() {
    _rawSocket.onPacket = std::bind(&TunnelServer::_rawSocketPacketHandler, this, std::placeholders::_1, std::placeholders::_2);
    _rawSocket.start(serverConfig.portFrom, serverConfig.portTo);
    boost::asio::post(_context, std::bind(&TunnelServer::_rawSocketLoopHandler, this));

    auto requestBuf = _bufferPool->alloc();
    requestBuf.moveFrontBoundary(50);

    _socket.async_receive_from(
      requestBuf.toMutableBuffer(),
      _receiveEp,
      std::bind(&TunnelServer::_onSocketReceive, this, std::placeholders::_1, std::placeholders::_2, requestBuf)
    );

    LOG_TRACE << fmt::format("tunnel server is running on port {}", serverConfig.listenPort);

    _context.run();
  }

  void TunnelServer::stop() {
    _rawSocket.stop();
    _context.stop();
  }

  void TunnelServer::_onSocketReceive(error_code err, std::size_t transfered, libtun::Buffer buf) {
    if (err.failed()) {
      _bufferPool->free(buf);
      return;
    }

    auto command = buf.data()[0];
    buf.size(transfered);
    buf.moveFrontBoundary(1);

    if (command == Command::TRANSMIT) {
      _processTransmit(buf);
    } else if (command == Command::REPLY || command == Command::REQUEST) {
      _rpc.feed(_receiveEp, buf);
    }
    _bufferPool->free(buf);

    auto requestBuf = _bufferPool->alloc();
    requestBuf.moveFrontBoundary(50);

    _socket.async_receive_from(
      requestBuf.toMutableBuffer(),
      _receiveEp,
      std::bind(&TunnelServer::_onSocketReceive, this, std::placeholders::_1, std::placeholders::_2, requestBuf)
    );
  }

  void TunnelServer::_processTransmit(const libtun::Buffer& buf) {
    uint16_t clientId = endian::big_to_native(*((uint16_t*)buf.data()));
    Ip4 ip4(buf.data() + 2, buf.size() - 2);

    if (
      clientId >= sessions.size() ||
      sessions[clientId].endpoint != _receiveEp ||
      !sessions[clientId].isConnect() ||
      ip4.calculateChecksum() != ip4.checksum()
    ) {
      return;
    }

    sessions[clientId].cryptor.decrypt(buf.data() + 2, buf.size() - 2);
    sessions[clientId].updateTransmit(buf.size() + 1);

    if (ip4.protocol() == Ip4::Protocol::TCP) {
      Tcp tcp(ip4);
      auto conn = tcpNapt.createIfNotExist(clientId, tcp.sourcePort(), ip4.destIP(), tcp.destPort());
      tcp.sourcePort(conn->clientPort);
      ip4.sourceIP(_rawSocket.ifAddress);
      tcp.calculateChecksumInplace();
      ip4.calculateChecksumInplace();
    } else if (ip4.protocol() == Ip4::Protocol::UDP) {
      Udp udp(ip4);
      auto conn = udpNapt.createIfNotExist(clientId, udp.sourcePort(), ip4.destIP(), udp.destPort());
      udp.sourcePort(conn->clientPort);
      ip4.sourceIP(_rawSocket.ifAddress);
      udp.checksum(0);
      ip4.calculateChecksumInplace();
    } else {
      return;
    }

    _rawSocket.write(buf.data() + 2, buf.size() - 2);
  }

  void TunnelServer::_removeSession(uint16_t id) {
    if (id <= sessions.size()) {
      sessions[id].status = SessionStatus::IDLE;
      tcpNapt.removeClient(id);
      udpNapt.removeClient(id);
    }
  }

  std::tuple<RpcErrorType, std::string, std::string> TunnelServer::_rpcConnectHandler(
    udp::endpoint from, std::string name, std::string password
  ) {
    if (name == "zxl" && password == "457348") {
      int16_t id = -1;
      for (int i = 0; i < sessions.size(); i++) {
        if (sessions[i].status == SessionStatus::IDLE) {
          id = i;
        }
      }

      if (id == -1) {
        id = sessions.size();
        sessions.push_back(Session(id));
      }

      sessions[id].clientId = id;
      sessions[id].cryptor = Cryptor();
      sessions[id].endpoint = from;
      sessions[id].lastActiveAt = system_clock::now();
      sessions[id].status = SessionStatus::CONNECTED;
      sessions[id].transmittedBytes = 0;
      sessions[id].error = RpcErrorType::SUCCESS;

      return {RpcErrorType::SUCCESS, sessions[id].cryptor.key, sessions[id].cryptor.iv};
    }
    return {RpcErrorType::WRONG_CREDENTIAL, "", ""};
  }

  RpcErrorType TunnelServer::_rpcPingHandler(udp::endpoint from) {
    for (int i = 0; i < sessions.size(); i++) {
      if (sessions[i].endpoint == from && sessions[i].status == SessionStatus::CONNECTED) {
        sessions[i].updateTransmit(0);
        return RpcErrorType::SUCCESS;
      }
    }
    return RpcErrorType::NOT_CONNECTED;
  }

  RpcErrorType TunnelServer::_rpcDisconnectHandler(udp::endpoint from) {
    for (int i = 0; i < sessions.size(); i++) {
      if (sessions[i].endpoint == from && sessions[i].status == SessionStatus::CONNECTED) {
        sessions[i].status = SessionStatus::IDLE;
        return RpcErrorType::SUCCESS;
      }
    }
    return RpcErrorType::NOT_CONNECTED;
  }

  void TunnelServer::_rawSocketLoopHandler() {
    _rawSocket.read();
    boost::asio::post(_context, std::bind(&TunnelServer::_rawSocketLoopHandler, this));
  }

  void TunnelServer::_rawSocketPacketHandler(uint8_t* data, uint32_t size) {
    Ip4 ip(data, size);
    uint16_t clientId;

    if (ip.protocol() == Ip4::Protocol::TCP) {
      Tcp tcp(ip);
      auto conn = tcpNapt.find(ip.sourceIP(), tcp.sourcePort(), tcp.destPort());
      if (!conn || sessions[conn->clientID].status != SessionStatus::CONNECTED) {
        return;
      }
      clientId = conn->clientID;
      tcp.destPort(conn->clientPort);
    } else if (ip.protocol() == Ip4::Protocol::UDP) {
      Udp udp(ip);
      auto conn = udpNapt.find(ip.sourceIP(), udp.sourcePort(), udp.destPort());
      if (!conn || sessions[conn->clientID].status != SessionStatus::CONNECTED) {
        return;
      }
      clientId = conn->clientID;
      udp.destPort(conn->clientPort);
    } else {
      return;
    }

    auto buf = _bufferPool->alloc();
    buf.moveFrontBoundary(10);
    buf.data()[0] = Command::TRANSMIT;
    buf.size(size + 1);

    sessions[clientId].cryptor.encrypt(data, buf.data() + 1, size);
    _socket.async_send_to(buf.toMutableBuffer(), sessions[clientId].endpoint, [buf, pool = _bufferPool](
      const error_code& sendErr, std::size_t transfered
    ) {
      pool->free(buf);
    });
  }

} // namespace znserver