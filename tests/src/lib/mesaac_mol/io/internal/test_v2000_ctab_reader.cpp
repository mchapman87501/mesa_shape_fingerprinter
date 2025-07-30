#include <catch2/catch_test_macros.hpp>

#include "ctab.hpp"
#include "mol_header_block.hpp"
#include "v2000_ctab_reader.hpp"

#include <sstream>

namespace mesaac::mol::internal {
namespace {
TEST_CASE("mesaac::mol::internal::V2000CTabReader - with properties block",
          "[mesaac]") {
  // This example is derived from the Biovia 2020 CTfile formats document.
  std::istringstream ins(R"LINES(  Bogus Structure
No metadata
No comments
  4  3  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    0.2136 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  CHG  1   1   4
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  MolHeaderBlock block;
  V2000CTabReader ctab_reader(reader);
  CTab actual;

  REQUIRE(block.read(reader));
  REQUIRE(ctab_reader.read(block, actual));
  REQUIRE(actual.atoms.size() == 4);
  REQUIRE(actual.bonds.size() == 3);
  REQUIRE(actual.raw_properties_block == "M  CHG  1   1   4\nM  END\n");
}

TEST_CASE("mesaac::mol::internal::V2000CTabReader - no properties",
          "[mesaac]") {
  std::istringstream ins(R"LINES(  Bogus Structure
No metadata
No comments
  4  3  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    0.2136 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  MolHeaderBlock block;
  V2000CTabReader ctab_reader(reader);
  CTab actual;

  REQUIRE(block.read(reader));
  REQUIRE(ctab_reader.read(block, actual));
  REQUIRE(actual.atoms.size() == 4);
  REQUIRE(actual.bonds.size() == 3);
  REQUIRE(actual.raw_properties_block == "M  END\n");
}

TEST_CASE("mesaac::mol::internal::V2000CTabReader - bad atom count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(  Bogus Structure
No metadata
No comments
  7  3  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    0.2136 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  MolHeaderBlock block;
  V2000CTabReader ctab_reader(reader);
  CTab actual;

  REQUIRE(block.read(reader));
  REQUIRE(!ctab_reader.read(block, actual));
  REQUIRE(actual.atoms.size() == 0);
  REQUIRE(actual.bonds.size() == 0);
  REQUIRE(actual.raw_properties_block == "");
}

TEST_CASE("mesaac::mol::internal::V2000CTabReader - bad bond count",
          "[mesaac]") {
  // V2000CTabReader is not guaranteed to fail on a too-low
  // bond count.  It may treat the extra bonds as properties.
  std::istringstream ins(R"LINES(  Bogus Structure
No metadata
No comments
  4  0  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    0.2136 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  MolHeaderBlock block;
  V2000CTabReader ctab_reader(reader);
  CTab actual;

  REQUIRE(block.read(reader));
  REQUIRE(!ctab_reader.read(block, actual));
  REQUIRE(actual.atoms.size() == 0);
  REQUIRE(actual.bonds.size() == 0);
  REQUIRE(actual.raw_properties_block == "");
}

} // namespace
} // namespace mesaac::mol::internal
