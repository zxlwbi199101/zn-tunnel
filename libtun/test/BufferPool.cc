#include <boost/test/unit_test.hpp>
#include <libtun/BufferPool.h>

using TunBuffer = libtun::Buffer;

BOOST_AUTO_TEST_SUITE(BufferPool)

  BOOST_AUTO_TEST_SUITE(Buffer)
    auto data = new uint8_t[20];

    BOOST_AUTO_TEST_CASE(default_constructor) {
      TunBuffer buf(data, 20);
      BOOST_REQUIRE_EQUAL(buf.data(), data);
      BOOST_REQUIRE_EQUAL(buf.size(), 20);
      BOOST_REQUIRE_EQUAL(buf.internal(), data);
      BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
      BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 0);
    }
    BOOST_AUTO_TEST_CASE(copy_constructor) {
      TunBuffer other(data, 20);
      TunBuffer buf = other;
      BOOST_REQUIRE_EQUAL(buf.data(), data);
      BOOST_REQUIRE_EQUAL(buf.size(), 20);
      BOOST_REQUIRE_EQUAL(buf.internal(), data);
      BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
      BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 0);
    }
    BOOST_AUTO_TEST_CASE(shift_constructor) {
      TunBuffer other(data, 20);
      TunBuffer buf(other, 3);
      BOOST_REQUIRE_EQUAL(buf.data(), data + 3);
      BOOST_REQUIRE_EQUAL(buf.size(), 17);
      BOOST_REQUIRE_EQUAL(buf.internal(), data);
      BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 3);
      BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 0);
    }

    BOOST_AUTO_TEST_CASE(shift_position) {
      TunBuffer buf(data, 20);
      BOOST_REQUIRE_EQUAL(buf.shift(3), 17);
      BOOST_REQUIRE_EQUAL(buf.data(), data + 3);
      BOOST_REQUIRE_EQUAL(buf.size(), 17);
      BOOST_REQUIRE_EQUAL(buf.internal(), data);
      BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 3);
      BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 0);

      BOOST_REQUIRE_EQUAL(buf.shift(17), 0);
      BOOST_REQUIRE_EQUAL(buf.shift(-20), 20);
      BOOST_REQUIRE_EQUAL(buf.shift(-1), 20);
      BOOST_REQUIRE_EQUAL(buf.shift(21), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
    }

    BOOST_AUTO_TEST_CASE(change_size) {
      TunBuffer buf(data, 20);
      BOOST_REQUIRE_EQUAL(buf.size(3), 3);
      BOOST_REQUIRE_EQUAL(buf.data(), data);
      BOOST_REQUIRE_EQUAL(buf.size(), 3);
      BOOST_REQUIRE_EQUAL(buf.internal(), data);
      BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
      BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 17);
    }

    BOOST_AUTO_TEST_CASE(to_const_buffer) {
      TunBuffer buf(data, 20);
      auto constBuffer = buf.toConstBuffer();
      BOOST_REQUIRE_EQUAL(constBuffer.data(), buf.data());
      BOOST_REQUIRE_EQUAL(constBuffer.size(), buf.size());
    }

    BOOST_AUTO_TEST_CASE(to_mutable_buffer) {
      TunBuffer buf(data, 20);
      auto mutableBuffer = buf.toMutableBuffer();
      BOOST_REQUIRE_EQUAL(mutableBuffer.data(), buf.data());
      BOOST_REQUIRE_EQUAL(mutableBuffer.size(), buf.size());
    }
  BOOST_AUTO_TEST_SUITE_END()

  // test cases for BufferPool
  libtun::BufferPool<200> pool(32);
  auto buf = pool.alloc();

  BOOST_AUTO_TEST_CASE(alloc) {
    BOOST_REQUIRE_EQUAL(pool.allCount(), 32);
    BOOST_REQUIRE_EQUAL(pool.consumedCount(), 1);
    BOOST_REQUIRE_EQUAL(pool.availableCount(), 31);
    BOOST_REQUIRE_EQUAL(buf.size(), 200);
    BOOST_REQUIRE_EQUAL(buf.internalSize(), 200);
  }

  BOOST_AUTO_TEST_CASE(free) {
    pool.free(buf);
    BOOST_REQUIRE_EQUAL(pool.allCount(), 32);
    BOOST_REQUIRE_EQUAL(pool.consumedCount(), 0);
    BOOST_REQUIRE_EQUAL(pool.availableCount(), 32);
  }

BOOST_AUTO_TEST_SUITE_END()
