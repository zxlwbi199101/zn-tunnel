#ifndef LIBTUN_TRANSMISSION_COMMUNICATOR_INCLUDED
#define LIBTUN_TRANSMISSION_COMMUNICATOR_INCLUDED

#include <string>
#include <boost/asio/ip/address_v4.hpp>
#include "./Cryptor.h"
#include "./Command.h"
#include "./Session.h"

namespace libtun {
namespace transmission {

  using boost::asio::mutable_buffer;

  inline int writeBuffer(const mutable_buffer& src, const mutable_buffer& dest, int& cur) {
    if (src.size() + cur + 1 > dest.size() || src.size() > 0xff) {
      throw;
    }
    auto data = (uint8_t*)(dest.data());
    data[cur++] = static_cast<uint8_t>(src.size());
    std::memcpy(data + cur, src.data(), src.size());
    cur += src.size();
    return src.size() + 1;
  }
  inline int writeString(const std::string& str, const mutable_buffer& dest, int& cur) {
    return writeBuffer(mutable_buffer((void*)str.data(), str.size()), dest, cur);
  }

  inline std::string readString(const mutable_buffer& dest, int& cur) {
    if (cur >= dest.size()) {
      throw;
    }
    auto data = (uint8_t*)(dest.data());
    auto len = data[cur++];

    if (len + cur > dest.size()) {
      throw;
    }
    cur += len;
    return std::string((const char*)(data + cur), len);
  }

  class Communicator {
  public:

    Communicator(const std::string& key, const std::string& iv) {
      _cryptor = Cryptor(key, iv);
    }

    void sendReqAuth(const mutable_buffer& buf, const std::string& username, const std::string& password) {
      auto data = (uint8_t*)(buf.data());
      int cur = 0;
      data[cur++] = Command::REQ_AUTH;
      writeString(username, buf, cur);
      writeString(password, buf, cur);

      _cryptor.encrypt(data + 1, cur - 1);
    }
    std::array<std::string, 2> receiveReqAuth(const mutable_buffer& buf) {
      _cryptor.decrypt((uint8_t*)(buf.data())+ 1, buf.size() - 1);
      int cur = 1;
      return {readString(buf, cur), readString(buf, cur)};
    }

    void sendResAuthSuccess(const mutable_buffer& buf, ClientSession& sess) {
      auto data = (uint8_t*)(buf.data());
      int cur = 0;
      data[cur++] = Command::RES_AUTH_SECCESS;
      writeString(sess.cryptor.key, buf, cur);
      writeString(sess.cryptor.iv, buf, cur);

      _cryptor.encrypt(data + 1, cur - 1);
    }
    std::array<std::string, 3> receiveResAuthSuccess(const mutable_buffer& buf) {
      _cryptor.decrypt((uint8_t*)(buf.data()) + 1, buf.size() - 1);
      int cur = 1;
      return {readString(buf, cur), readString(buf, cur), readString(buf, cur)};
    }

  private:
    Cryptor _cryptor;

  };

} // namespace transmission
} // namespace libtun

#endif