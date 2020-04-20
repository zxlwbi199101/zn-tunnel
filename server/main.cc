#include <libtun/BufferPool.h>
#include "TunnelServer.h"

int main() {

  libtun::BufferPool<1600> pool;
  znserver::TunnelServerConfig config = {
    .listenPort = 8080,
    .portFrom = 64335,
    .portTo = 64995,
    .maxSessions = 10,
    .key = "1234567890123456",
    .iv = "6543210987654321",
  };
  znserver::TunnelServer server(config, &pool);

  try {
    server.start();
  } catch (std::exception& e) {
    LOG_FATAL << e.what();
    return 1;
  }

  return 0;
}
