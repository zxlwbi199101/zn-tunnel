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

    std::vector<libtun::Buffer> requests;
    std::vector<libtun::Buffer> replies;
    int receiveCount = 0;

    Communicator(asio::io_context* context, Cryptor* cryptor, BufferPool<1600>* pool, int port, std::string debug):
      _context(context),
      _cryptor(cryptor),
      _pool(pool),
      _socket(*context, udp::endpoint(udp::v4(), port)),
      _rpc(_context, &_socket, _cryptor),
      _debug(debug) {

      _rpc.onRequest = std::bind(&Communicator::onRequest, this, std::placeholders::_1, std::placeholders::_2);

      _receiveBuf = _pool->alloc();
      _receiveBuf.shift(10);
      _socket.async_receive_from(
        _receiveBuf.toMutableBuffer(),
        _endpoint,
        std::bind(&Communicator::onReceive, this, std::placeholders::_1, std::placeholders::_2)
      );
    }

    void onReceive(boost::system::error_code err, std::size_t transfered) {
      _receiveBuf.size(transfered);
      _rpc.feed(_endpoint, _receiveBuf);

      _socket.async_receive_from(
        _receiveBuf.toMutableBuffer(),
        _endpoint,
        std::bind(&Communicator::onReceive, this, std::placeholders::_1, std::placeholders::_2)
      );
    }

    bool onRequest(libtun::Buffer buf, Rpc::ControlBlock* control) {
      requests.push_back(buf);

      auto buffer = _pool->alloc();
      buffer.shift(10);
      buffer.data()[0] = 3;
      buffer.data()[1] = 4;
      buffer.size(2);

      control->buffer = buffer;
      control->onComplete = [](boost::system::error_code err, libtun::Buffer buf, Rpc::ControlBlock* control) {};

      return true;
    }

    void send() {
      auto buffer = _pool->alloc();
      buffer.shift(10);
      buffer.data()[0] = 1;
      buffer.data()[1] = 2;
      buffer.size(2);

      _rpc.send(udp::endpoint(udp::v4(), 10060), buffer, [&](boost::system::error_code err, libtun::Buffer buf, Rpc::ControlBlock* control) {
        if (!err.failed()) replies.push_back(buf);
      });
    }

  private:

    asio::io_context* _context;
    Cryptor* _cryptor;
    BufferPool<1600>* _pool;
    udp::socket _socket;
    Rpc _rpc;
    udp::endpoint _endpoint;

    libtun::Buffer _receiveBuf;

    std::string _debug;
  };

  BOOST_AUTO_TEST_CASE(communication) {
    asio::io_context context;
    Cryptor cryptor;
    BufferPool<1600> pool;

    Communicator server(&context, &cryptor, &pool, 10060, "server");
    Communicator client(&context, &cryptor, &pool, 10061, "client");
    client.send();

    asio::steady_timer timer(context);
    timer.expires_after(std::chrono::seconds(3));
    timer.async_wait([&context](boost::system::error_code err) {
      context.stop();
    });

    context.run();

    BOOST_REQUIRE_EQUAL(server.requests.size(), 1);
    BOOST_REQUIRE_EQUAL(client.replies.size(), 1);
  }

BOOST_AUTO_TEST_SUITE_END()
