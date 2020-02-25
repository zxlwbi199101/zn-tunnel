#include <boost/test/unit_test.hpp>
#include <libtun/protocol.h>
#include "../_utils/utils.h"

BOOST_AUTO_TEST_SUITE(protocol_udp)

  using libtun::protocol::Ip4;
  using libtun::protocol::Udp;

  auto sampleUDP = readFile("test/.data/ip_udp_1");

  BOOST_AUTO_TEST_CASE(get_values) {
    Udp udp((Ip4(sampleUDP)));
    BOOST_REQUIRE_EQUAL(udp.sourcePort(), 4500);
    BOOST_REQUIRE_EQUAL(udp.destPort(), 4500);
    BOOST_REQUIRE_EQUAL(udp.totalLen(), 640);
    BOOST_REQUIRE_EQUAL(udp.checksum(), 0);
  }

BOOST_AUTO_TEST_SUITE_END()
