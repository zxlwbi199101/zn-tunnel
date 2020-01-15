#include <thread>
#include <signal.h>
#include <libtun/logger.h>
#include <libtun/Tunnel.h>
// #include "./parser/ParseConfig.h"

// #define DEFAULT_INI "src/config/Autoconfig.ini"

void _sigHandler(int signum) {
  libtun::closeDumps();
  exit(1);
}

int main(int argc, char **argv) {
  // pcx::ParseConfig config;
  // config.ReadConfig(DEFAULT_INI);
  // libtun::enableFileLog(config.ReadString("LoggerStart", "dir", "/Users/edison/Desktop/zntun%N.log").c_str());
  // libtun::enableConsoleLog();
  // signal(SIGINT, _sigHandler);

  libtun::enableFileLog("/Users/xzhao048/Desktop/zntunnel%N.log");
  libtun::enableConsoleLog();
  libtun::Tunnel client;
  libtun::Tunnel server;
  if (!client.open() || !server.open()) {
    exit(1);
  }
  // std::string dir = config.ReadString("LoggerPackage", "dir", "/Users/edison/Desktop/ip");
  while (1) {
    auto buffer = client.read();

    libtun::dumpBuffer(buffer.data, buffer.len, "/Users/xzhao048/Desktop/IP.dump");
    libtun::dumpBuffer((uint8_t*)"IP_PACKET_END", 13, "/Users/xzhao048/Desktop/IP.dump");
  }

  return 0;
}
