#include <boost/test/unit_test.hpp>
#include <libtun/protocol.h>
#include "../_utils/utils.h"

BOOST_AUTO_TEST_SUITE(protocol_ip4)

  using libtun::protocol::Ip4;

  auto sampleUDP = readFile("test/.data/ip_udp_1");

  BOOST_AUTO_TEST_CASE(get_values) {
    Ip4 ip(sampleUDP);
    BOOST_REQUIRE_EQUAL(ip.version(), 4);
    BOOST_REQUIRE_EQUAL(ip.headerLen(), 5);
    BOOST_REQUIRE_EQUAL(ip.totalLen(), 660);
    BOOST_REQUIRE_EQUAL(ip.id(), 20799);
    BOOST_REQUIRE_EQUAL(ip.flag(), Ip4::Flag::NONE);
    BOOST_REQUIRE_EQUAL(ip.offset(), 0);
    BOOST_REQUIRE_EQUAL(ip.ttl(), 64);
    BOOST_REQUIRE_EQUAL(ip.protocol(), Ip4::Protocol::UDP);
    BOOST_REQUIRE_EQUAL(ip.checksum(), 38881);
    BOOST_REQUIRE_EQUAL(ip.sourceIP().to_string(), "192.168.1.6");
    BOOST_REQUIRE_EQUAL(ip.destIP().to_string(), "188.166.16.228");
    BOOST_REQUIRE_EQUAL(ip.payload().size(), 640);
  }

  BOOST_AUTO_TEST_CASE(calculate_checksum) {
    Ip4 ip(sampleUDP);
    BOOST_REQUIRE_EQUAL(ip.checksum(), ip.calculateChecksum());
  }

  BOOST_AUTO_TEST_CASE(set_values) {
    auto address1 = boost::asio::ip::make_address_v4("15.16.17.18");
    auto address2 = boost::asio::ip::make_address_v4("255.254.253.252");
    Ip4 ip(sampleUDP);
    ip.checksum(0xfeab);
    ip.sourceIP(address1);
    ip.destIP(address2);

    Ip4 newIp(sampleUDP);
    BOOST_REQUIRE_EQUAL(newIp.checksum(), 0xfeab);
    BOOST_REQUIRE_EQUAL(newIp.sourceIP(), address1);
    BOOST_REQUIRE_EQUAL(newIp.destIP(), address2);
  }

BOOST_AUTO_TEST_SUITE_END()
