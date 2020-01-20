#include <boost/test/unit_test.hpp>
#include <libtun/Exception.h>

BOOST_AUTO_TEST_SUITE(Exception)

  BOOST_AUTO_TEST_CASE(throwable) {
    BOOST_REQUIRE_EXCEPTION(
      throw libtun::Exception("message"),
      std::exception,
      [](const std::exception& e) {
        return std::string(e.what()) == "message";
      }
    );
  }

  BOOST_AUTO_TEST_CASE(use_right_value) {
    std::string content = "exception message";
    libtun::Exception e(std::move(content));
    BOOST_REQUIRE_EQUAL(e.what(), "exception message");
    BOOST_REQUIRE_EQUAL("", content);
  }

BOOST_AUTO_TEST_SUITE_END()
