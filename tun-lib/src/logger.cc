#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <fstream>
#include <signal.h>

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

void enableFileLog(const char *path, int rotateInMB = 10) {
  logging::add_file_log(
    keywords::file_name = path,
    keywords::rotation_size = rotateInMB * 1024 * 1024,
    keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
    keywords::format = "[%TimeStamp%][%Severity%]: %Message%",
    keywords::open_mode = std::ios_base::app
  );
  logging::add_common_attributes();
}

void enableConsoleLog() {
  logging::add_console_log(
    std::cout,
    boost::log::keywords::format = "[%TimeStamp%][%Severity%]: %Message%"
  );
}

std::ofstream dumpFile;

void _sigHandler(int signum) {
  if (dumpFile.is_open()) {
    dumpFile.close();
  }
  exit(1);
}

void dumpIpPacket(const uint8_t* buf, int len) {
  if (!dumpFile.is_open()) {
    dumpFile.open("/Users/xzhao048/Desktop/ipdump.binlog", std::ofstream::binary | std::ofstream::app);
    signal(SIGINT, _sigHandler);
  }

  if (dumpFile.is_open()) {
    dumpFile.write((char*)buf, len);
  }
}
