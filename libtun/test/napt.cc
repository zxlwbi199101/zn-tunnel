#include <boost/test/unit_test.hpp>
#include <libtun/napt.h>

BOOST_AUTO_TEST_SUITE(napt)

  using boost::asio::ip::address_v4;
  using boost::asio::ip::make_address_v4;

  BOOST_AUTO_TEST_CASE(create_connection_and_find) {
    libtun::NAPT<address_v4, address_v4> tcpNAPT(100, 200);
    auto clientIP = make_address_v4("192.168.1.1");
    auto serverIP = make_address_v4("69.171.229.11");
    auto conn = tcpNAPT.createIfNotExist(clientIP, 52000, serverIP, 443);
    BOOST_REQUIRE_EQUAL(conn->clientIP.to_string(), clientIP.to_string());
    BOOST_REQUIRE_EQUAL(conn->clientPort, 52000);
    BOOST_REQUIRE_EQUAL(conn->serverIP.to_string(), serverIP.to_string());
    BOOST_REQUIRE_EQUAL(conn->serverPort, 443);
    BOOST_REQUIRE_EQUAL(conn->localPort, 100);

    auto connC = tcpNAPT.find(clientIP, 52000);
    auto connS = tcpNAPT.find(serverIP, 443, 100);
    BOOST_REQUIRE_EQUAL(connC.get(), conn.get());
    BOOST_REQUIRE_EQUAL(connS.get(), conn.get());
  }

BOOST_AUTO_TEST_SUITE_END()
