#include <boost/test/unit_test.hpp>
#include <nlohmann/json.hpp>
#include <libtun/transmission/RpcBuffer.h>
#include <libtun/transmission/Cryptor.h>

using nlohmann::json;
using libtun::Buffer;
using libtun::transmission::Cryptor;
using libtun::transmission::RpcBuffer;
using libtun::BufferPool;

BOOST_AUTO_TEST_SUITE(protocol_transmission_rpcbuffer)

  uint8_t data[20];
  Buffer originBuf(data, 20);
  Cryptor cryptor;
  BufferPool<1600> pool;

  BOOST_AUTO_TEST_CASE(default_constructor) {
    RpcBuffer buf(originBuf, &cryptor, false);
    BOOST_REQUIRE_EQUAL(buf.data(), data);
    BOOST_REQUIRE_EQUAL(buf.size(), 20);
    BOOST_REQUIRE_EQUAL(buf.internal(), data);
    BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
    BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
    BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 0);
  }

  BOOST_AUTO_TEST_CASE(copy_constructor) {
    RpcBuffer other(originBuf, &cryptor, false);
    RpcBuffer buf = other;
    BOOST_REQUIRE_EQUAL(buf.data(), data);
    BOOST_REQUIRE_EQUAL(buf.size(), 20);
    BOOST_REQUIRE_EQUAL(buf.internal(), data);
    BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
    BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
    BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 0);
  }

  BOOST_AUTO_TEST_CASE(setters_and_getters) {
    RpcBuffer buf(originBuf, &cryptor, false);
    buf
      .command(libtun::transmission::Command::TRANSMIT)
      .id(9527)
      .content({ { "a", 1 } });
    BOOST_REQUIRE_EQUAL(buf.command(), libtun::transmission::Command::TRANSMIT);
    BOOST_REQUIRE_EQUAL(buf.id(), 9527);
    BOOST_REQUIRE_EQUAL(buf.content().dump(), "{\"a\":1}");
  }

  // BOOST_AUTO_TEST_CASE(to_const_buffer) {
  //   RpcBuffer buf(data, 20);
  //   auto constBuffer = buf.toConstBuffer();
  //   BOOST_REQUIRE_EQUAL(constBuffer.data(), buf.data());
  //   BOOST_REQUIRE_EQUAL(constBuffer.size(), buf.size());
  // }

  // BOOST_AUTO_TEST_CASE(to_mutable_buffer) {
  //   RpcBuffer buf(data, 20);
  //   auto mutableBuffer = buf.toMutableBuffer();
  //   BOOST_REQUIRE_EQUAL(mutableBuffer.data(), buf.data());
  //   BOOST_REQUIRE_EQUAL(mutableBuffer.size(), buf.size());
  // }

BOOST_AUTO_TEST_SUITE_END()
