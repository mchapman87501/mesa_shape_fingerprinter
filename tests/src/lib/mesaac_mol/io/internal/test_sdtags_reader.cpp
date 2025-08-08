#include <catch2/catch_test_macros.hpp>

#include "sdtags_reader.hpp"

#include <sstream>

namespace mesaac::mol::internal {
namespace {
TEST_CASE("mesaac::mol::internal::SDTagsReader - Basics", "[mesaac]") {
  std::istringstream ins(R"LINES(>  <Name>
A Chemical Structure

>  <Family>
A.1

>  <IC50_uM>
0.06

>  <set>
1

$$$$)LINES");

  LineReader reader(ins, "<from a string>");
  SDTagsReader tags_reader(reader);
  const auto reader_result = tags_reader.read();

  REQUIRE(reader_result.is_ok());

  const std::map<std::string, std::string> expected{
      {">  <Name>", "A Chemical Structure\n"},
      {">  <Family>", "A.1\n"},
      {">  <IC50_uM>", "0.06\n"},
      {">  <set>", "1\n"},
  };
  const auto &actual = reader_result.value();
  REQUIRE(actual == expected);
}

TEST_CASE("mesaac::mol::internal::SDTagsReader - Missing terminator",
          "[mesaac]") {
  std::istringstream ins(R"LINES(>  <Name>
anon

>  <IC50_uM>
0.06

)LINES");

  LineReader reader(ins, "<from a string>");
  SDTagsReader tags_reader(reader);

  const auto reader_result = tags_reader.read();
  // TODO capture cerr from .read.
  REQUIRE(!reader_result.is_ok());
}
} // namespace
} // namespace mesaac::mol::internal