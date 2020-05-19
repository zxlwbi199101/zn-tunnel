#include <thread>
#include <signal.h>
#include <libtun/logger.h>
#include <libtun/Tunnel.h>

void _sigHandler(int signum) {
  exit(1);
}

int main(int argc, char **argv) {

  return 0;
}
