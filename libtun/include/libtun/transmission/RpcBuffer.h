#include <nlohmann/json.hpp>
#include "../BufferPool.h"
#include "./Cryptor.h"
#include "./constant.h"

namespace libtun {
namespace transmission {

  using nlohmann::json;

  /* A RPC PACKET
  ---------------------------------------------------------
  |command |       id       |       len      | json string
  ---------------------------------------------------------
  */

  class RpcBuffer: public Buffer {
  public:

    RpcBuffer(const Buffer& buf, Cryptor* cryptor, bool encrypted = true):
      Buffer(buf), _cryptor(cryptor), _encrypted(encrypted) {}

    RpcBuffer(const RpcBuffer& other):
      Buffer(other), _cryptor(other._cryptor), _encrypted(other._encrypted) {}

    // getters
    const_buffer toConstBuffer() {
      _encrypt();
      return const_buffer(data(), size());
    }

    mutable_buffer toMutableBuffer() {
      _encrypt();
      return mutable_buffer(data(), size());
    }

    Command command() { return (Command)readNumber<uint8_t>(0); }

    uint16_t id() {
      _decrypt();
      return readNumber<uint16_t>(1);
    }

    json content() {
      _decrypt();
      uint16_t len = readNumber<uint16_t>(3);

      if (len + 5 > size()) {
        return json::value_t::null;
      }

      std::string jsonStr((const char*)(data() + 5), len);
      return json::parse(jsonStr);
    }

    bool isValid() {
      if (size() < 5) {
        return false;
      }

      uint16_t len = readNumber<uint16_t>(3);
      return size() - 5 >= len;
    }

    // setters
    RpcBuffer& command(Command cmd) {
      writeNumber<uint8_t>(cmd, 0);
      return *this;
    }

    RpcBuffer& id(uint16_t i) {
      _decrypt();
      if (size() < 3) {
        size(3);
      }
      writeNumber<uint16_t>(i, 1);
      return *this;
    }

    RpcBuffer& content(const json& j) {
      _decrypt();
      std::string jsonStr = j.dump();
      uint16_t len = jsonStr.length();

      if (len + 5 <= internalSize() - prefixSpace()) {
        size(len + 5);
        writeNumber<uint16_t>(len, 3);
        std::memcpy(data() + 5, jsonStr.c_str(), len);
      }

      return *this;
    }

  private:

    Cryptor* _cryptor;
    bool _encrypted;

    void _encrypt() {
      if (!_encrypted) {
        _cryptor->encrypt(data() + 1, size() - 1);
        _encrypted = true;
      }
    }

    void _decrypt() {
      if (_encrypted) {
        _cryptor->decrypt(data() + 1, size() - 1);
        _encrypted = false;
      }
    }

  };

} // namespace transmission
} // namespace libtun
