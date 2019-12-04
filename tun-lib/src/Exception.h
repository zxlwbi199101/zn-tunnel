#ifndef TUN_LIB_EXCEPTION_INCLUDED
#define TUN_LIB_EXCEPTION_INCLUDED

#include <exception>
#include <string>

namespace tunlib {

  class Exception: public std::exception {
  public:
    Exception(const std::string& msg): _msg(msg) {}
    Exception(std::string&& msg): _msg(std::move(msg)) {}

    virtual const char* what() const noexcept {
      return _msg.c_str();
    };

    virtual ~Exception() throw() {}

  private:
    std::string _msg;
  };

}

#endif