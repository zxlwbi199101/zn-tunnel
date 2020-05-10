#ifndef LIBTUN_TRANSMISSION_CRYPTOR_INCLUDED
#define LIBTUN_TRANSMISSION_CRYPTOR_INCLUDED

#include <string>
#include <boost/asio/buffer.hpp>
#include <cryptopp/osrng.h>
#include <cryptopp/aes.h>
#include <cryptopp/modes.h>

namespace libtun {
namespace transmission {

  using boost::asio::mutable_buffer;

  class Cryptor {
  public:

    typedef CryptoPP::byte Byte;

    std::string key;
    std::string iv;

    Cryptor(const std::string& _key, const std::string& _iv) {
      key = _key;
      iv = _iv;
      _encryption.SetKeyWithIV((Byte*)(key.data()), key.size(), (Byte*)(iv.data()));
      _decryption.SetKeyWithIV((Byte*)(key.data()), key.size(), (Byte*)(iv.data()));
    }

    Cryptor() {
      key.resize(CryptoPP::AES::DEFAULT_KEYLENGTH);
      iv.resize(CryptoPP::AES::DEFAULT_KEYLENGTH);

      CryptoPP::AutoSeededRandomPool random;
      random.GenerateBlock((Byte*)(key.data()), key.size());
      random.GenerateBlock((Byte*)(iv.data()), iv.size());
      _encryption.SetKeyWithIV((Byte*)(key.data()), key.size(), (Byte*)(iv.data()));
      _decryption.SetKeyWithIV((Byte*)(key.data()), key.size(), (Byte*)(iv.data()));
    }

    Cryptor(const Cryptor& other):
      Cryptor(other.key, other.iv) {}

    void encrypt(void* data, uint32_t size) {
      _encryption.ProcessData((Byte*)data, (Byte*)data, size);
      _encryption.Resynchronize((Byte*)(iv.data()), iv.size());
    }
    void encrypt(void* src, void* dest, uint32_t size) {
      _encryption.ProcessData((Byte*)dest, (Byte*)src, size);
      _encryption.Resynchronize((Byte*)(iv.data()), iv.size());
    }

    void decrypt(void* data, uint32_t size) {
      _decryption.ProcessData((Byte*)data, (Byte*)data, size);
      _decryption.Resynchronize((Byte*)(iv.data()), iv.size());
    }
    void decrypt(void* src, void* dest, uint32_t size) {
      _decryption.ProcessData((Byte*)dest, (Byte*)src, size);
      _decryption.Resynchronize((Byte*)(iv.data()), iv.size());
    }

  private:
    CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption _encryption;
    CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption _decryption;
  };


} // namespace transmission
} // namespace libtun

#endif