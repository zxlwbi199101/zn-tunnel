#pragma once

#include <string>
#include <random>
#include <tiny-AES-c/aes.hpp>

namespace libtun {
namespace transmission {

  class Cryptor {
  public:

    std::string key;
    std::string iv;

    Cryptor(const std::string& _key, const std::string& _iv) {
      key = _key;
      iv = _iv;
      AES_init_ctx(&_ctx, (uint8_t*)key.data());
    }

    Cryptor() {
      std::default_random_engine generator;
      std::uniform_int_distribution<unsigned short> distribution(0, 255);

      key.resize(16);
      iv.resize(16);

      for (uint8_t i = 0; i < 16; i++) {
        key.data()[i] = (char)(distribution(generator));
        iv.data()[i] = (char)(distribution(generator));
      }

      AES_init_ctx(&_ctx, (uint8_t*)key.data());
    }

    Cryptor(const Cryptor& other):
      Cryptor(other.key, other.iv) {}

    void encrypt(uint8_t* data, uint32_t size) {
      AES_ctx_set_iv(&_ctx, (uint8_t*)iv.data());
      AES_CBC_encrypt_buffer(&_ctx, data, size);
    }

    void decrypt(uint8_t* data, uint32_t size) {
      AES_ctx_set_iv(&_ctx, (uint8_t*)iv.data());
      AES_CBC_decrypt_buffer(&_ctx, data, size);
    }

  private:

    struct AES_ctx _ctx;

  };


} // namespace transmission
} // namespace libtun
