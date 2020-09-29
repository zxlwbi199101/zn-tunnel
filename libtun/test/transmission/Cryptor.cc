#include <string>
#include <boost/test/unit_test.hpp>
#include <libtun/transmission/Cryptor.h>

BOOST_AUTO_TEST_SUITE(protocol_transmission_cryptor)

  std::string data = "this is my data";

  BOOST_AUTO_TEST_CASE(default_constructor) {
    std::string str(data);
    libtun::transmission::Cryptor cryptor;
    cryptor.encrypt((uint8_t*)(str.data()), str.size());
    cryptor.decrypt((uint8_t*)(str.data()), str.size());
    BOOST_REQUIRE_EQUAL(str, data);
  }

  BOOST_AUTO_TEST_CASE(copy_constructor) {
    std::string str1(data);
    std::string str2(data);

    libtun::transmission::Cryptor cryptor;
    libtun::transmission::Cryptor copyCryptor = cryptor;
    BOOST_REQUIRE_EQUAL(cryptor.key, copyCryptor.key);
    BOOST_REQUIRE_EQUAL(cryptor.iv, copyCryptor.iv);

    cryptor.encrypt((uint8_t*)(str1.data()), str1.size());
    copyCryptor.encrypt((uint8_t*)(str2.data()), str2.size());
    BOOST_REQUIRE_EQUAL(str1, str2);
    cryptor.decrypt((uint8_t*)(str1.data()), str1.size());
    copyCryptor.decrypt((uint8_t*)(str2.data()), str2.size());
    BOOST_REQUIRE_EQUAL(str1, data);
    BOOST_REQUIRE_EQUAL(str2, data);
  }

  BOOST_AUTO_TEST_CASE(cryptor_reuse) {
    std::string str1(data);
    std::string str2(data);
    libtun::transmission::Cryptor cryptor;

    cryptor.encrypt((uint8_t*)(str1.data()), str1.size());
    cryptor.encrypt((uint8_t*)(str2.data()), str2.size());

    BOOST_REQUIRE_EQUAL(str1, str2);

    cryptor.decrypt((uint8_t*)(str1.data()), str1.size());
    cryptor.decrypt((uint8_t*)(str2.data()), str2.size());

    BOOST_REQUIRE_EQUAL(str1, str2);
    BOOST_REQUIRE_EQUAL(str1, data);
  }

BOOST_AUTO_TEST_SUITE_END()
