#include <thread>
#include "./logger.h"
#include "./tun/Tun.h"
#include <signal.h>
#include "./parser/ParseConfig.h"
#define DEFAULT_INI "src/config/Autoconfig.ini"

void _sigHandler(int signum) {
  tunlib::closeDumps();
  exit(1);
}

int main(int argc, char **argv) {
  pcx::ParseConfig config;
  config.ReadConfig(DEFAULT_INI);
  tunlib::enableFileLog(config.ReadString("LoggerStart", "dir", "/Users/edison/Desktop/zntun%N.log").c_str());
  tunlib::enableConsoleLog();
  signal(SIGINT, _sigHandler);

  tunlib::Tun client;
  tunlib::Tun server;
  if (!client.open() || !server.open()) {
    exit(1);
  }
  std::string dir = config.ReadString("LoggerPackage", "dir", "/Users/edison/Desktop/ip");
  while (1) {
    auto buffer = client.read();

    tunlib::dumpBuffer(buffer.data, buffer.len, dir.c_str());
    tunlib::dumpBuffer((uint8_t*)"IP_PACKET_END", 13, dir.c_str());
  }

  return 0;
}
