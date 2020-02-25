#include <vector>
#include <net/bpf.h>
#include <net/ethernet.h>
#include <sys/socket.h>
#include <sys/ioctl.h>

#include <boost/asio.hpp>
#include <libtun/logger.h>
#include <libtun/protocol/ip4.h>

namespace asio = boost::asio;
using asio::mutable_buffer;
using asio::detail::socket_addr_type;
using asio::ip::udp;

int main() {
  try {
    asio::io_context context;
    // asio::ip::udp::udp_server server(io_context);
    // io_context.run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  return 0;
}
