#include <catch2/catch_test_macros.hpp>

#include "line_reader.hpp"

#include <format>
#include <sstream>

namespace mesaac::mol::internal {
namespace {
TEST_CASE("mesaac::mol::internal::LineReader - General", "[mesaac]") {
  std::istringstream ins(R"LINES(Content 1
Content 2
Content 3)LINES");

  LineReader reader(ins, "<from a string>");
  std::string line;

  REQUIRE(reader.file_pos().find("<from a string>") != std::string::npos);
  REQUIRE(reader.file_pos().find("line 0") != std::string::npos);
  for (size_t i = 1; i < 4; ++i) {
    REQUIRE(reader.next(line));
    REQUIRE(line == std::format("Content {}", i));
    const auto line_num_expr = std::format("line {}", i);
    REQUIRE(reader.file_pos().find(line_num_expr) != std::string::npos);
  }
  REQUIRE(!reader.next(line));
}

} // namespace
} // namespace mesaac::mol::internal