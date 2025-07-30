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
  SDTagMap actual;

  REQUIRE(tags_reader.read(actual));

  const std::map<std::string, std::string> expected{
      {">  <Name>", "A Chemical Structure\n"},
      {">  <Family>", "A.1\n"},
      {">  <IC50_uM>", "0.06\n"},
      {">  <set>", "1\n"},
  };

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
  SDTagMap actual;

  // TODO capture cerr from .read.
  REQUIRE(!tags_reader.read(actual));

  // The false return value indicates the missing '$$$$'.
  // But the map should still have parsed the content.
  const std::map<std::string, std::string> expected{
      {">  <Name>", "anon\n"},
      {">  <IC50_uM>", "0.06\n"},
  };

  REQUIRE(actual == expected);
}
} // namespace
} // namespace mesaac::mol::internal