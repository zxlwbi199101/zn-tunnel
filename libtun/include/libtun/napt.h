#ifndef LIBTUN_NAPT_INCLUDED
#define LIBTUN_NAPT_INCLUDED

#include <unordered_map>
#include <string>
#include <array>
#include <memory>
#include <boost/container_hash/hash.hpp>
#include <boost/asio.hpp>

namespace libtun {

  // NON THREAD SAFE
  template<class IPAddress, class StateMachine>
  class NAPT {
  public:

    struct Connection {
      IPAddress serverIP;
      uint16_t serverPort;
      IPAddress clientIP;
      uint16_t clientPort;
      uint16_t localPort;
      StateMachine stateMachine;
    };

    struct ServerKey {
      IPAddress serverIP;
      uint16_t serverPort;
      uint16_t localPort;
      bool operator == (const ServerKey& k) const {
        return serverIP == k.serverIP && serverPort == k.serverPort && localPort == k.localPort;
      }
    };
    struct ServerKeyHash {
      size_t operator() (const ServerKey& k) const {
        size_t seed = 0;
        for (auto i : k.serverIP.to_bytes()) {
          boost::hash_combine(seed, i);
        }
        boost::hash_combine(seed, k.serverPort);
        return seed;
      }
    };

    struct ClientKey {
      IPAddress clientIP;
      uint16_t clientPort;
      bool operator == (const ClientKey& k) const {
        return clientIP == k.clientIP && clientPort == k.clientPort;
      }
    };
    struct ClientKeyHash {
      size_t operator() (const ClientKey& k) const {
        size_t seed = 0;
        for (auto i : k.clientIP.to_bytes()) {
          boost::hash_combine(seed, i);
        }
        boost::hash_combine(seed, k.clientPort);
        return seed;
      }
    };

    typedef std::shared_ptr<Connection> ConnectionPtr;

    NAPT(uint16_t availablePortFrom, uint16_t availablePortTo) {
      _portFrom = availablePortFrom;
      _portTo = availablePortTo;
    }

    ConnectionPtr find(const IPAddress& serverIP, uint16_t serverPort, uint16_t localPort) const {
      struct ServerKey key = {
        .serverIP = serverIP,
        .serverPort = serverPort,
        .localPort = localPort,
      };
      auto it = _serverMap.find(key);
      if (it == _serverMap.end()) {
        return nullptr;
      }
      return it->second;
    }

    ConnectionPtr find(const IPAddress& clientIP, uint16_t clientPort) const {
      struct ClientKey key = {
        .clientIP = clientIP,
        .clientPort = clientPort,
      };
      auto it = _clientMap.find(key);
      if (it == _clientMap.end()) {
        return nullptr;
      }
      return it->second;
    }

    ConnectionPtr createIfNotExist(
      const IPAddress& clientIP,
      uint16_t clientPort,
      const IPAddress& serverIP,
      uint16_t serverPort
    ) {
      auto conn = find(clientIP, clientPort);
      if (conn != nullptr && conn->serverIP == serverIP && conn->serverPort == serverPort) {
        return conn;
      }

      ClientKey ck = {
        .clientIP = clientIP,
        .clientPort = clientPort,
      };
      ServerKey sk = {
        .serverIP = serverIP,
        .serverPort = serverPort,
      };
      for (uint16_t p = _portFrom; p <= _portTo; p++) {
        sk.localPort = p;
        if (_serverMap.find(sk) == _serverMap.end()) {
          break;
        }
      }

      if (sk.localPort < _portFrom || sk.localPort > _portTo) {
        return nullptr;
      }

      ConnectionPtr ptr(new Connection());
      ptr->serverIP = serverIP;
      ptr->serverPort = serverPort;
      ptr->localPort = sk.localPort;
      ptr->clientIP = clientIP;
      ptr->clientPort = clientPort;

      _serverMap[sk] = ptr;
      _clientMap[ck] = ptr;

      return ptr;
    }

  private:
    uint16_t _portFrom;
    uint16_t _portTo;
    std::unordered_map<ServerKey, ConnectionPtr, ServerKeyHash> _serverMap;
    std::unordered_map<ClientKey, ConnectionPtr, ClientKeyHash> _clientMap;
  };

} // namespace libtun

#endif