#ifndef LIBTUN_LOGGER_INCLUDED
#define LIBTUN_LOGGER_INCLUDED

#include <string>
#include <fstream>
#include <boost/log/expressions.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/trivial.hpp>
#include <boost/filesystem.hpp>
#include <fmt/core.h>
#include "./Exception.h"

#define LOG_TRACE BOOST_LOG_TRIVIAL(trace)
#define LOG_DEBUG BOOST_LOG_TRIVIAL(debug)
#define LOG_INFO BOOST_LOG_TRIVIAL(info)
#define LOG_WARNING BOOST_LOG_TRIVIAL(warning)
#define LOG_ERROR BOOST_LOG_TRIVIAL(error)
#define LOG_FATAL BOOST_LOG_TRIVIAL(fatal)

namespace libtun {

  namespace logging = boost::log;
  namespace sinks = boost::log::sinks;
  namespace src = boost::log::sources;
  namespace expr = boost::log::expressions;
  namespace attrs = boost::log::attributes;
  namespace keywords = boost::log::keywords;
  namespace fs = boost::filesystem;

  inline void _createFileByFullPath(const std::string& fullPath) {
    fs::path p(fullPath);

    if (!p.is_absolute()) {
      LOG_FATAL << "requires an absolute full path.";
      throw Exception("requires an absolute full path.");
    }
    if (p.has_parent_path() && !fs::exists(p.parent_path())) {
      fs::create_directories(p.parent_path());
      LOG_INFO << fmt::format("directory {} not exists, created.", p.parent_path().string());
    }
  }

  inline void enableConsoleLog() {
    logging::add_console_log(
      std::cout,
      keywords::format = "[%TimeStamp%][%Severity%]: %Message%"
    );
  }

  inline void enableFileLog(const std::string& fullPath, int rotateInMB) {
    _createFileByFullPath(fullPath);

    logging::add_file_log(
      keywords::file_name = fullPath,
      keywords::rotation_size = rotateInMB * 1024 * 1024,
      keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
      keywords::format = "[%TimeStamp%][%Severity%]: %Message%",
      keywords::open_mode = std::ios_base::app
    );
    logging::add_common_attributes();
  }

  class BinLogger {
  public:
    BinLogger(const std::string& fullPath, const std::string& debugName = "") {
      _createFileByFullPath(fullPath);

      this->debugName = debugName;
      this->file.open(fullPath, std::ofstream::binary | std::ofstream::app);

      if (!file.is_open()) {
        LOG_ERROR << fmt::format("[{}] binlog [{}] open failed.", debugName, fullPath);
      } else {
        LOG_INFO << fmt::format("[{}] binlog file is {}.", debugName, fullPath);
      }
    }

    void dump(const void* buf, uint32_t len) {
      if (file.is_open()) {
        file.write((char*)buf, len).flush();
      }
    }

    ~BinLogger() {
      if (file.is_open()) {
        file.close();
      }
    }

  private:
    std::string debugName;
    std::ofstream file;
  };

}

#endif