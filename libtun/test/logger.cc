#include <fstream>
#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include <libtun/logger.h>

namespace fs = boost::filesystem;

BOOST_AUTO_TEST_SUITE(file_log)

  BOOST_AUTO_TEST_CASE(auto_create_log_file) {
    auto p = fs::current_path() / "log/a.binlog";
    auto binlogger = new libtun::BinLogger(p.string());

    BOOST_REQUIRE_EQUAL(fs::exists(p), true);

    binlogger->dump("content", 7);
    std::string content;
    std::getline(std::ifstream(p.string()), content);
    BOOST_REQUIRE_EQUAL(content, "content");

    delete binlogger;
    fs::remove_all(fs::current_path() / "log");
  }

  BOOST_AUTO_TEST_CASE(throw_relative_path) {
    BOOST_REQUIRE_EXCEPTION(
      libtun::BinLogger("a.binlog"),
      std::exception,
      [](const std::exception& e) {
        return std::string(e.what()) == "requires an absolute full path.";
      }
    );
  }

BOOST_AUTO_TEST_SUITE_END()
