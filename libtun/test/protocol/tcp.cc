#include <boost/test/unit_test.hpp>
#include <libtun/protocol.h>
#include "../_utils/utils.h"

BOOST_AUTO_TEST_SUITE(protocol_tcp)

  using libtun::protocol::Ip4;
  using libtun::protocol::Tcp;

  auto sampleTCP = readFile("test/.data/ip_tcp_1");

  BOOST_AUTO_TEST_CASE(get_values) {
    Tcp tcp((Ip4(sampleTCP)));
    BOOST_REQUIRE_EQUAL(tcp.sourcePort(), 443);
    BOOST_REQUIRE_EQUAL(tcp.destPort(), 62052);
    BOOST_REQUIRE_EQUAL(tcp.sequence(), 116688936);
    BOOST_REQUIRE_EQUAL(tcp.acknowledgment(), 698757062);
    BOOST_REQUIRE_EQUAL(tcp.headerLen(), 5);
    BOOST_REQUIRE_EQUAL(tcp.headerLen(), 5);
    BOOST_REQUIRE_EQUAL(tcp.CWR(), false);
    BOOST_REQUIRE_EQUAL(tcp.ECE(), false);
    BOOST_REQUIRE_EQUAL(tcp.URG(), false);
    BOOST_REQUIRE_EQUAL(tcp.ACK(), true);
    BOOST_REQUIRE_EQUAL(tcp.PSH(), false);
    BOOST_REQUIRE_EQUAL(tcp.RST(), false);
    BOOST_REQUIRE_EQUAL(tcp.SYN(), false);
    BOOST_REQUIRE_EQUAL(tcp.FIN(), false);
    BOOST_REQUIRE_EQUAL(tcp.window(), 2655);
    BOOST_REQUIRE_EQUAL(tcp.checksum(), 37712);
    BOOST_REQUIRE_EQUAL(tcp.urgentPointer(), 0);
  }

  BOOST_AUTO_TEST_CASE(calculate_checksum) {
    Tcp tcp((Ip4(sampleTCP)));
    BOOST_REQUIRE_EQUAL(tcp.checksum(), tcp.calculateChecksum());
  }

  BOOST_AUTO_TEST_CASE(set_values) {
    Tcp tcp((Ip4(sampleTCP)));
    tcp.sourcePort(0xffff);
    tcp.destPort(0xfeff);
    tcp.checksum(0xfefe);

    BOOST_REQUIRE_EQUAL(tcp.sourcePort(), 0xffff);
    BOOST_REQUIRE_EQUAL(tcp.destPort(), 0xfeff);
    BOOST_REQUIRE_EQUAL(tcp.checksum(), 0xfefe);
  }

BOOST_AUTO_TEST_SUITE_END()
