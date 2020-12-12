#include <string>
#include <functional>
#include <vector>
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/system/error_code.hpp>
#include <fmt/core.h>
#include <libtun/transmission.h>

BOOST_AUTO_TEST_SUITE(protocol_transmission_rpc_protocol)

  namespace asio = boost::asio;
  using asio::ip::udp;
  using libtun::transmission::Cryptor;
  using libtun::transmission::RpcProtocol;
  using libtun::transmission::RpcErrorType;
  using libtun::BufferPool;

  class Communicator {
  public:

    std::vector<std::string> socketReceives;
    std::vector<std::string> requests;
    std::vector<std::string> replies;
    udp::endpoint endpointFrom;
    libtun::Buffer socketBuf;
    uint8_t internalBuf[1600];

    Communicator(asio::io_context* context, Cryptor* cryptor, BufferPool<1600>* pool, int port):
      _context(context),
      _cryptor(cryptor),
      _pool(pool),
      _socket(*context, udp::endpoint(udp::v4(), port)),
      _rpc(_context, &_socket, _cryptor, pool, 3) {

      _rpc.onConnect = [&](udp::endpoint from, std::string name, std::string pwd) {
        requests.push_back(fmt::format("connect:{}_{}", name, pwd));
        return std::make_tuple(RpcErrorType::SUCCESS, "key", "iv");
      };
      _rpc.onPing = [&](udp::endpoint from) {
        requests.push_back("ping");
        return RpcErrorType::INVALID_INPUT;
      };
      _rpc.onDisconnect = [&](udp::endpoint from) {
        requests.push_back("disconnect");
        return RpcErrorType::SUCCESS;
      };

      socketBuf = libtun::Buffer(internalBuf, 1600);
      socketBuf.moveFrontBoundary(10);
      _socket.async_receive_from(
        socketBuf.toMutableBuffer(),
        endpointFrom,
        std::bind(&Communicator::onReceive, this, std::placeholders::_1, std::placeholders::_2, socketBuf)
      );
    }

    void onReceive(boost::system::error_code err, std::size_t transfered, libtun::Buffer buf) {
      socketReceives.push_back(std::string((const char*)buf.data(), transfered));

      buf.size(transfered);
      _rpc.feed(endpointFrom, buf);

      _socket.async_receive_from(
        socketBuf.toMutableBuffer(),
        endpointFrom,
        std::bind(&Communicator::onReceive, this, std::placeholders::_1, std::placeholders::_2, socketBuf)
      );
    }

    void send() {
      _rpc.connect(udp::endpoint(udp::v4(), 10060), "name", "pwd", [&](RpcErrorType err, std::string key, std::string iv) {
        replies.push_back(fmt::format("connect:{}_{}_{}", (uint8_t)err, key, iv));
      });
      _rpc.ping(udp::endpoint(udp::v4(), 10060), [&](RpcErrorType err) {
        replies.push_back(fmt::format("ping:{}", (uint8_t)err));
      });
      _rpc.disconnect(udp::endpoint(udp::v4(), 10060), [&](RpcErrorType err) {
        replies.push_back(fmt::format("disconnect:{}", (uint8_t)err));
      });
    }

  private:
    asio::io_context* _context;
    Cryptor* _cryptor;
    BufferPool<1600>* _pool;
    udp::socket _socket;
    RpcProtocol _rpc;

  };

  BOOST_AUTO_TEST_CASE(rpc_protocol_communication) {
    asio::io_context context;
    Cryptor cryptor;
    BufferPool<1600> pool;

    Communicator server(&context, &cryptor, &pool, 10060);
    Communicator client(&context, &cryptor, &pool, 10061);
    client.send();

    asio::steady_timer timer(context);
    timer.expires_after(std::chrono::seconds(4));
    timer.async_wait([&](boost::system::error_code err) {
      context.stop();
    });

    context.run();

    // BOOST_REQUIRE_EQUAL(server.socketReceives.size(), 1);
    // BOOST_REQUIRE_EQUAL(server.socketReceives[0].size(), 5);

    // BOOST_REQUIRE_EQUAL(client.socketReceives.size(), 2);
    // BOOST_REQUIRE_EQUAL(client.socketReceives[0].size(), 5);
    // BOOST_REQUIRE_EQUAL(client.socketReceives[0], client.socketReceives[1]);

    BOOST_REQUIRE_EQUAL(server.requests.size(), 3);
    BOOST_REQUIRE_EQUAL(server.requests[0], "connect:name_pwd");
    BOOST_REQUIRE_EQUAL(server.requests[1], "ping");
    BOOST_REQUIRE_EQUAL(server.requests[2], "disconnect");

    BOOST_REQUIRE_EQUAL(client.replies.size(), 3);
    BOOST_REQUIRE_EQUAL(client.replies[0], "connect:0_key_iv");
    BOOST_REQUIRE_EQUAL(client.replies[1], "ping:1");
    BOOST_REQUIRE_EQUAL(client.replies[2], "disconnect:0");

    BOOST_REQUIRE_EQUAL(pool.consumedCount(), 0);
    BOOST_REQUIRE_EQUAL(pool.availableCount(), 32);
  }

BOOST_AUTO_TEST_SUITE_END()
