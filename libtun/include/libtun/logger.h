#ifndef LIBTUN_LOGGER_INCLUDED
#define LIBTUN_LOGGER_INCLUDED

#include <boost/log/trivial.hpp>
#include <string>

#define trace BOOST_LOG_TRIVIAL(trace)
#define debug BOOST_LOG_TRIVIAL(debug)
#define info BOOST_LOG_TRIVIAL(info)
#define warning BOOST_LOG_TRIVIAL(warning)
#define error BOOST_LOG_TRIVIAL(error)
#define fatal BOOST_LOG_TRIVIAL(fatal)

namespace libtun {

  void enableConsoleLog();
  void enableFileLog(const std::string& path, int rotateInMB = 10);

  // void setDumpDir(const std::string& dir);
  void dumpBuffer(const uint8_t* buf, int len, const std::string& dumpName);
  void closeDumps();

}

#endif