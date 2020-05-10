#include <boost/test/unit_test.hpp>
#include <libtun/BufferPool.h>

using TunBuffer = libtun::Buffer;

BOOST_AUTO_TEST_SUITE(BufferPool)

  BOOST_AUTO_TEST_SUITE(Buffer)
    uint8_t data[20];

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

    BOOST_AUTO_TEST_CASE(shift) {
      TunBuffer buf(data, 20);
      buf.size(10);
      BOOST_REQUIRE_EQUAL(buf.shift(3), 10);
      BOOST_REQUIRE_EQUAL(buf.data() - data, 3);
      BOOST_REQUIRE_EQUAL(buf.data(), data + 3);
      BOOST_REQUIRE_EQUAL(buf.size(), 10);

      BOOST_REQUIRE_EQUAL(buf.shift(8), 10);
      BOOST_REQUIRE_EQUAL(buf.data(), data + 3);
      BOOST_REQUIRE_EQUAL(buf.size(), 10);

      BOOST_REQUIRE_EQUAL(buf.shift(-2), 10);
      BOOST_REQUIRE_EQUAL(buf.data(), data + 1);
      BOOST_REQUIRE_EQUAL(buf.size(), 10);
    }

    BOOST_AUTO_TEST_CASE(move_front_boundary) {
      TunBuffer buf(data, 20);
      BOOST_REQUIRE_EQUAL(buf.moveFrontBoundary(3), 17);
      BOOST_REQUIRE_EQUAL(buf.data(), data + 3);
      BOOST_REQUIRE_EQUAL(buf.size(), 17);
      BOOST_REQUIRE_EQUAL(buf.internal(), data);
      BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 3);
      BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 0);

      BOOST_REQUIRE_EQUAL(buf.moveFrontBoundary(17), 0);
      BOOST_REQUIRE_EQUAL(buf.moveFrontBoundary(-20), 20);
      BOOST_REQUIRE_EQUAL(buf.moveFrontBoundary(-1), 20);
      BOOST_REQUIRE_EQUAL(buf.moveFrontBoundary(21), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
    }

    BOOST_AUTO_TEST_CASE(move_back_boundary) {
      TunBuffer buf(data, 20);
      BOOST_REQUIRE_EQUAL(buf.moveBackBoundary(-3), 17);
      BOOST_REQUIRE_EQUAL(buf.data(), data);
      BOOST_REQUIRE_EQUAL(buf.size(), 17);
      BOOST_REQUIRE_EQUAL(buf.internal(), data);
      BOOST_REQUIRE_EQUAL(buf.internalSize(), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
      BOOST_REQUIRE_EQUAL(buf.suffixSpace(), 3);

      BOOST_REQUIRE_EQUAL(buf.moveBackBoundary(-17), 0);
      BOOST_REQUIRE_EQUAL(buf.moveBackBoundary(20), 20);
      BOOST_REQUIRE_EQUAL(buf.moveBackBoundary(1), 20);
      BOOST_REQUIRE_EQUAL(buf.moveBackBoundary(-21), 20);
      BOOST_REQUIRE_EQUAL(buf.prefixSpace(), 0);
    }

    BOOST_AUTO_TEST_CASE(write_read_buffer) {
      TunBuffer buf(data, 20);
      buf.size(0);
      uint8_t content[5] = {1, 2, 3, 4, 5};

      BOOST_REQUIRE_EQUAL(buf.writeBuffer(content, 5, 0), true);
      BOOST_REQUIRE_EQUAL(buf.size(), 7);
      BOOST_REQUIRE_EQUAL(data[0], 0);
      BOOST_REQUIRE_EQUAL(data[1], 5);
      BOOST_REQUIRE_EQUAL_COLLECTIONS(content, content + 5, data + 2, data + 7);
      
      auto readContent = buf.readBuffer(0);
      BOOST_REQUIRE_EQUAL(readContent.size(), 5);
      BOOST_REQUIRE_EQUAL_COLLECTIONS(content, content + 5, (uint8_t*)readContent.data(), (uint8_t*)readContent.data() + 5);

      BOOST_REQUIRE_EQUAL(buf.writeBuffer(content, 5, 16), false);
    }

    BOOST_AUTO_TEST_CASE(write_read_string) {
      TunBuffer buf(data, 20);
      buf.size(0);
      std::string content = "hello";

      BOOST_REQUIRE_EQUAL(buf.writeString(content, 0), true);
      BOOST_REQUIRE_EQUAL(buf.size(), 7);
      BOOST_REQUIRE_EQUAL(data[0], 0);
      BOOST_REQUIRE_EQUAL(data[1], 5);
      
      std::string readContent = buf.readString(0);
      BOOST_REQUIRE_EQUAL(readContent, content);

      BOOST_REQUIRE_EQUAL(buf.writeString(content, 16), false);
    }

    BOOST_AUTO_TEST_CASE(write_read_front_back) {
      TunBuffer buf(data, 20);
      buf.size(0);
      std::string content = "hello";
      
      BOOST_REQUIRE_EQUAL(buf.writeStringToBack(content), true);
      BOOST_REQUIRE_EQUAL(buf.writeStringToBack(content), true);
      BOOST_REQUIRE_EQUAL(buf.size(), 14);
      BOOST_REQUIRE_EQUAL(buf.readStringFromFront(), content);
      BOOST_REQUIRE_EQUAL(buf.readStringFromFront(), content);
      BOOST_REQUIRE_EQUAL(buf.size(), 0);
      BOOST_REQUIRE_EQUAL(buf.readStringFromFront(), "");
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
