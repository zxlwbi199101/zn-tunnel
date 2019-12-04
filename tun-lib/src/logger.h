#ifndef TUN_LIB_LOGGER_INCLUDED
#define TUN_LIB_LOGGER_INCLUDED

#include <boost/log/trivial.hpp>

#define trace BOOST_LOG_TRIVIAL(trace)
#define debug BOOST_LOG_TRIVIAL(debug)
#define info BOOST_LOG_TRIVIAL(info)
#define warning BOOST_LOG_TRIVIAL(warning)
#define error BOOST_LOG_TRIVIAL(error)
#define fatal BOOST_LOG_TRIVIAL(fatal)

namespace tunlib {

  void enableFileLog(const char *path, int rotateInMB = 10);
  void enableConsoleLog();

  void dumpBuffer(const uint8_t* buf, int len, std::string&& dumpName);
  void closeDumps();

}

#endif