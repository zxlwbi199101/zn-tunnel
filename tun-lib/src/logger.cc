#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <fstream>
#include <memory>
#include <string>
#include <unordered_map>

#include "./logger.h"

namespace logging = boost::log;
namespace sinks = boost::log::sinks;
namespace src = boost::log::sources;
namespace expr = boost::log::expressions;
namespace attrs = boost::log::attributes;
namespace keywords = boost::log::keywords;

namespace tunlib {

  void enableFileLog(const char *path, int rotateInMB) {
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

  typedef std::shared_ptr<std::ofstream> streamPtr;
  std::unordered_map<std::string, streamPtr> binLogs;

  void dumpBuffer(const uint8_t* buf, int len, std::string&& dumpName) {
    streamPtr logFile;

    if (binLogs.find(dumpName) == binLogs.end()) {
      logFile = streamPtr(new std::ofstream(
        "/Users/xzhao048/Desktop/" + dumpName,
        std::ofstream::binary | std::ofstream::app
      ));
      binLogs.insert({ dumpName, logFile });
    } else {
      logFile = binLogs[dumpName];
    }

    if (!logFile->is_open()) {
      error << "cannot open dump file: " << dumpName;
    } else {
      logFile->write((char*)buf, len);
    }
  }

  void closeDumps() {
    for (auto& kv : binLogs) {
      kv.second->close();
    }
    binLogs.clear();
  }

}