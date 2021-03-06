#include <string>
#include <functional>
#include <vector>
#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/system/error_code.hpp>
#include <libtun/transmission.h>

BOOST_AUTO_TEST_SUITE(protocol_transmission_rpc)

  namespace asio = boost::asio;
  using asio::ip::udp;
  using libtun::transmission::Cryptor;
  using libtun::transmission::Rpc;
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
      _rpc(_context, &_socket, _cryptor, 3) {

      _rpc.onRequest = [&](libtun::Buffer buf, Rpc::ControlBlock* control) {
        requests.push_back(std::string((const char*)buf.data(), buf.size()));

        auto buffer = _pool->alloc();
        buffer.moveFrontBoundary(10);
        buffer.data()[0] = 3;
        buffer.data()[1] = 4;
        buffer.size(2);

        control->buffer = buffer;
        control->onComplete = [&](boost::system::error_code err, libtun::Buffer buf, Rpc::ControlBlock* control) {
          _pool->free(buf);
        };

        return true;
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
      auto buffer = _pool->alloc();
      buffer.moveFrontBoundary(10);
      buffer.data()[0] = 1;
      buffer.data()[1] = 2;
      buffer.size(2);

      _rpc.send(udp::endpoint(udp::v4(), 10060), buffer, [&](boost::system::error_code err, libtun::Buffer buf, Rpc::ControlBlock* control) {
        if (!err.failed()) {
          replies.push_back(std::string((const char*)buf.data(), buf.size()));
        }
        _pool->free(control->buffer);
      });
    }

  private:
    asio::io_context* _context;
    Cryptor* _cryptor;
    BufferPool<1600>* _pool;
    udp::socket _socket;
    Rpc _rpc;

  };

  BOOST_AUTO_TEST_CASE(communication) {
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

    BOOST_REQUIRE_EQUAL(server.socketReceives.size(), 1);
    BOOST_REQUIRE_EQUAL(server.socketReceives[0].size(), 5);

    BOOST_REQUIRE_EQUAL(client.socketReceives.size(), 2);
    BOOST_REQUIRE_EQUAL(client.socketReceives[0].size(), 5);
    BOOST_REQUIRE_EQUAL(client.socketReceives[0], client.socketReceives[1]);

    BOOST_REQUIRE_EQUAL(server.requests.size(), 1);
    BOOST_REQUIRE_EQUAL(server.requests[0], "\1\2");

    BOOST_REQUIRE_EQUAL(client.replies.size(), 1);
    BOOST_REQUIRE_EQUAL(client.replies[0], "\3\4");

    BOOST_REQUIRE_EQUAL(pool.consumedCount(), 0);
    BOOST_REQUIRE_EQUAL(pool.availableCount(), 32);
  }

BOOST_AUTO_TEST_SUITE_END()
