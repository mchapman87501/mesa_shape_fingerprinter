#include <catch2/catch_test_macros.hpp>

#include "mol_header_block.hpp"

#include <format>
#include <sstream>

namespace mesaac::mol::internal {
namespace {
TEST_CASE("mesaac::mol::internal::MolHeaderBlock - Valid V3000 Counts Line",
          "[mesaac]") {
  std::istringstream ins(R"LINES(Molecule Name
metadata fields go here
No comment
  0  0  0     0  0            999 V3000)LINES");

  LineReader reader(ins, "<from a string>");
  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  REQUIRE(block.is_v3000());
  REQUIRE(block.name() == "Molecule Name");
  REQUIRE(block.metadata() == "metadata fields go here");
  REQUIRE(block.comments() == "No comment");
  REQUIRE(block.counts_line().size() > 0);
}

TEST_CASE("mesaac::mol::internal::MolHeaderBlock - Valid V2000 Counts Line",
          "[mesaac]") {
  std::istringstream ins(R"LINES(Molecule Name
metadata fields go here
No comment
 20 40  0     0  0            999 V2000)LINES");

  LineReader reader(ins, "<from a string>");
  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  REQUIRE(!block.is_v3000());
  REQUIRE(block.name() == "Molecule Name");
  REQUIRE(block.metadata() == "metadata fields go here");
  REQUIRE(block.comments() == "No comment");
  REQUIRE(block.counts_line().size() > 0);
}
} // namespace
} // namespace mesaac::mol::internal
