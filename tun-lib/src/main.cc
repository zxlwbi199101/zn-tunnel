#include <thread>
#include "./logger.h"
#include "./tun/Tun.h"
#include <signal.h>

void _sigHandler(int signum) {
  tunlib::closeDumps();
  exit(1);
}

int main(int argc, char **argv) {
  tunlib::enableFileLog("/Users/xzhao048/Desktop/zntun%N.log");
  tunlib::enableConsoleLog();
  signal(SIGINT, _sigHandler);

  tunlib::Tun client;
  tunlib::Tun server;
  if (!client.open() || !server.open()) {
    exit(1);
  }

  while (1) {
    auto buffer = client.read();

    tunlib::dumpBuffer(buffer.data, buffer.len, "ip");
    tunlib::dumpBuffer((uint8_t*)"IP_PACKET_END", 13, "ip");
  }

  return 0;
}
