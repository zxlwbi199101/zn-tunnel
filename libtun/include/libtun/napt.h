#ifndef LIBTUN_NAPT_INCLUDED
#define LIBTUN_NAPT_INCLUDED

#include <map>
#include <unordered_map>
#include <string>
#include <boost/container_hash/hash.hpp>
#include <boost/pool/object_pool.hpp>

namespace libtun {

  using boost::object_pool;

  // NOT THREAD SAFE
  template<class IPAddress, class StateMachine>
  class NAPT {
  public:

    struct Connection {
      IPAddress serverIP;
      uint16_t serverPort;
      uint16_t clientID;
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
      uint16_t clientID;
      uint16_t clientPort;
      bool operator == (const ClientKey& k) const {
        return clientID == k.clientID && clientPort == k.clientPort;
      }
      bool operator < (const ClientKey& k) const {
        if (clientID == k.clientID) return clientPort < k.clientPort;
		    return clientID == k.clientID;
	}

    };
    struct ClientKeyHash {
      size_t operator() (const ClientKey& k) const {
        size_t seed = 0;
        boost::hash_combine(seed, k.clientID);
        boost::hash_combine(seed, k.clientPort);
        return seed;
      }
    };

    NAPT() {}
    NAPT(const NAPT& other) {
      _portFrom = other._portFrom;
      _portTo = other._portTo;
    }
    NAPT(uint16_t availablePortFrom, uint16_t availablePortTo) {
      _portFrom = availablePortFrom;
      _portTo = availablePortTo;
    }

    Connection* find(const IPAddress& serverIP, uint16_t serverPort, uint16_t localPort) const {
      auto it = _serverMap.find({
        .serverIP = serverIP,
        .serverPort = serverPort,
        .localPort = localPort,
      });
      if (it == _serverMap.end()) {
        return nullptr;
      }
      return it->second;
    }

    Connection* find(uint16_t clientID, uint16_t clientPort) const {
      auto it = _clientMap.find({
        .clientID = clientID,
        .clientPort = clientPort,
      });
      if (it == _clientMap.end()) {
        return nullptr;
      }
      return it->second;
    }

    Connection* createIfNotExist(
      uint16_t clientID,
      uint16_t clientPort,
      const IPAddress& serverIP,
      uint16_t serverPort
    ) {
      auto conn = find(clientID, clientPort);
      if (conn != nullptr && conn->serverIP == serverIP && conn->serverPort == serverPort) {
        return conn;
      }

      ClientKey ck = {
        .clientID = clientID,
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

      Connection* ptr = _pool.malloc();
      ptr->serverIP = serverIP;
      ptr->serverPort = serverPort;
      ptr->localPort = sk.localPort;
      ptr->clientID = clientID;
      ptr->clientPort = clientPort;

      _serverMap[sk] = ptr;
      _clientMap[ck] = ptr;

      return ptr;
    }

    void removeClient(uint8_t clientID) {
      auto it = _clientMap.upper_bound({
        .clientID = clientID,
        .clientPort = 0,
      });

      while (it != _clientMap.end()) {
        if (it->first.clientID != clientID) {
          break;
        }
        _serverMap.erase({
          .serverIP = it->second->serverIP,
          .serverPort = it->second->serverPort,
          .localPort = it->second->localPort,
        });
        _clientMap.erase(it++);
      }
    }

  private:
    uint16_t _portFrom;
    uint16_t _portTo;
    std::unordered_map<ServerKey, Connection*, ServerKeyHash> _serverMap;
    std::map<ClientKey, Connection*> _clientMap;
    object_pool<Connection> _pool;
  };

} // namespace libtun

#endif