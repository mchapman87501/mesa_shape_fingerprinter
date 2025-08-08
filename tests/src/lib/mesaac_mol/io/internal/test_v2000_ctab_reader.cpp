#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

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
  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V2000CTabReader ctab_reader(reader);

  const auto ctab_result = ctab_reader.read(block);

  REQUIRE(ctab_result.is_ok());

  const auto actual = ctab_result.value();
  REQUIRE(actual.atoms.size() == 4);
  REQUIRE(actual.bonds.size() == 3);
  REQUIRE(actual.atoms.at(0).props().chg == 4);
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

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V2000CTabReader ctab_reader(reader);

  const auto ctab_result = ctab_reader.read(block);

  REQUIRE(ctab_result.is_ok());

  const auto actual = ctab_result.value();
  REQUIRE(actual.atoms.size() == 4);
  REQUIRE(actual.bonds.size() == 3);
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

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V2000CTabReader ctab_reader(reader);
  const auto ctab_result = ctab_reader.read(block);

  REQUIRE(!ctab_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000CTabReader - bond count too large",
          "[mesaac]") {
  // V2000CTabReader is not guaranteed to fail on a too-low
  // bond count.  It may treat the extra bonds as properties.
  // This test is for a too-large bond count.
  std::istringstream ins(R"LINES(  Bogus Structure
No metadata
No comments
  4  8  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    0.2136 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  END)LINES");

  LineReader reader(ins, "<from a string>");

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V2000CTabReader ctab_reader(reader);
  const auto ctab_result = ctab_reader.read(block);

  REQUIRE(!ctab_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000CTabReader - negative atom count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(Negative atom count
No metadata
No comments
  -4  3  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    0.2136 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  END)LINES");

  LineReader reader(ins, "<negative atom count>");

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V2000CTabReader ctab_reader(reader);
  const auto ctab_result = ctab_reader.read(block);

  REQUIRE(!ctab_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000CTabReader - negative bond count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(Negative bond count
No metadata
No comments
  4 -3  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    0.2136 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  END)LINES");

  LineReader reader(ins, "<negative bond count>");

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V2000CTabReader ctab_reader(reader);
  const auto ctab_result = ctab_reader.read(block);

  REQUIRE(!ctab_result.is_ok());
}

TEST_CASE(
    "mesaac::mol::internal::V2000CTabReader - non-default atom block fields",
    "[mesaac]") {
  std::istringstream ins(R"LINES(  Bogus Structure
No metadata
No comments
  4  3  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C  -3  2  3  2  1 12  1  0  0  3  2  1
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    0.2136 C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  END)LINES");

  LineReader reader(ins, "<from a string>");

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V2000CTabReader ctab_reader(reader);

  const auto ctab_result = ctab_reader.read(block);

  REQUIRE(ctab_result.is_ok());

  const auto actual = ctab_result.value();
  REQUIRE(actual.atoms.size() == 4);
  REQUIRE(actual.bonds.size() == 3);

  const auto &atom = actual.atoms.at(0);
  const auto &props = atom.props();
  // Carbon mass -3:
  REQUIRE_THAT(props.mass, Catch::Matchers::WithinAbs(12.011f - 3, 1.0e-6));
  REQUIRE(props.chg == 2);
  REQUIRE(props.cfg == 3);
  REQUIRE(props.hcount == 2);
  REQUIRE(props.stbox == 1);
  REQUIRE(props.val == 12);
  REQUIRE(props.aamap == 3);
  REQUIRE(props.invret == 2);
  REQUIRE(props.exachg == 1);
}

TEST_CASE("mesaac::mol::internal::V2000CTabReader - bad coords", "[mesaac]") {
  std::istringstream ins(R"LINES(  Bad Coordinates
No metadata
No comments
  4  3  0     0  0  0  0  0  0999 V2000
   -1.0004    2.0110    0.0442 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.7085    0.8754    0.7938 C   0  0  0  0  0  0  0  0  0  0  0  0
   -1.4618   -0.4721    0.1588 C   0  0  0  0  0  0  0  0  0  0  0  0
   -0.1768   -1.0440    X_XXXX C   0  0  0  0  0  0  0  0  0  0  0  0
  1  2  1  0  0  0  0
  2  3  1  0  0  0  0
  3  4  1  0  0  0  0
M  END)LINES");

  LineReader reader(ins, "<from a string>");

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V2000CTabReader ctab_reader(reader);

  const auto ctab_result = ctab_reader.read(block);

  REQUIRE(!ctab_result.is_ok());
}

} // namespace
} // namespace mesaac::mol::internal
