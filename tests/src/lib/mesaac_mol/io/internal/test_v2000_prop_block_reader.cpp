#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mesaac_mol/atom.hpp"
#include "mesaac_mol/bond.hpp"
#include "v2000_prop_block_reader.hpp"

#include <sstream>

namespace mesaac::mol::internal {
namespace {

TEST_CASE("mesaac::mol::internal::V2000PropBlockReader - CHG", "[mesaac]") {
  std::istringstream ins(R"LINES(M  CHG  1   1   4
M  END)LINES");

  LineReader line_reader(ins, "<from a string>");
  AtomVector atoms{
      Atom({.atomic_num = 6, .props = {.chg = -1}}),
      Atom({.atomic_num = 8, .props = {.chg = 2}}),
  };

  REQUIRE(atoms.at(0).props().chg == -1);
  REQUIRE(atoms.at(1).props().chg == 2);

  BondVector bonds;

  V2000PropBlockReader pb_reader(line_reader, atoms, bonds);

  const auto read_result = pb_reader.read();
  REQUIRE(read_result.is_ok());

  // Verify that the CHG charge was applied to atom 0, and that the
  // charge of atom 1 was reset.
  REQUIRE(atoms.at(0).props().chg == 4);
  REQUIRE(atoms.at(1).props().chg == 0);
}

TEST_CASE("mesaac::mol::internal::V2000PropBlockReader - Unsupported property",
          "[mesaac]") {
  // V2000 format allows to specify ring bond counts, but
  // the current Mesa code does not support this.
  // It doesn't flag the input as an error.  It does print a warning - which
  // this test does not capture/verify.
  std::istringstream ins(R"LINES(M  RBC  1   2   1
M  END)LINES");

  LineReader line_reader(ins, "<from a string>");
  AtomVector atoms;
  BondVector bonds;

  V2000PropBlockReader pb_reader(line_reader, atoms, bonds);

  const auto read_result = pb_reader.read();
  REQUIRE(read_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000PropBlockReader - Invalid prefix",
          "[mesaac]") {
  // Unrecognized property lines are discarded, with a warning.
  // This test does not capture/verify the warning.
  std::istringstream ins(R"LINES(O  ISO  1   2   1
M  END)LINES");

  LineReader line_reader(ins, "<from a string>");
  AtomVector atoms;
  BondVector bonds;

  V2000PropBlockReader pb_reader(line_reader, atoms, bonds);

  const auto read_result = pb_reader.read();
  REQUIRE(read_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000PropBlockReader - bad count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(M  CHG  X   1   4
M  END)LINES");

  LineReader line_reader(ins, "<from a string>");
  AtomVector atoms;
  BondVector bonds;

  V2000PropBlockReader pb_reader(line_reader, atoms, bonds);

  const auto read_result = pb_reader.read();
  REQUIRE(!read_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000PropBlockReader - bad atom index",
          "[mesaac]") {
  std::istringstream ins(R"LINES(M  CHG  1   X   4
M  END)LINES");

  LineReader line_reader(ins, "<from a string>");
  AtomVector atoms;
  BondVector bonds;

  V2000PropBlockReader pb_reader(line_reader, atoms, bonds);

  const auto read_result = pb_reader.read();
  REQUIRE(!read_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000PropBlockReader - bad chg/rad value",
          "[mesaac]") {
  std::istringstream ins(R"LINES(M  RAD  1   1   X
M  END)LINES");

  LineReader line_reader(ins, "<from a string>");
  AtomVector atoms;
  BondVector bonds;

  V2000PropBlockReader pb_reader(line_reader, atoms, bonds);

  const auto read_result = pb_reader.read();
  REQUIRE(!read_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000PropBlockReader - bad atom index (zero)",
          "[mesaac]") {
  std::istringstream ins(R"LINES(M  RAD  2   1   1   0   2
M  END)LINES");

  LineReader line_reader(ins, "<from a string>");
  AtomVector atoms{
      Atom({.atomic_num = 6, .props = {.chg = -1}}),
      Atom({.atomic_num = 8, .props = {.chg = 2}}),
  };
  BondVector bonds;

  V2000PropBlockReader pb_reader(line_reader, atoms, bonds);

  const auto read_result = pb_reader.read();
  REQUIRE(!read_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V2000PropBlockReader - RAD", "[mesaac]") {
  std::istringstream ins(R"LINES(M  RAD  2   1   3   2   1
M  END)LINES");

  LineReader line_reader(ins, "<from a string>");
  // I'm not sure explicit chg + explicit rad makes any sense.
  // The combination is used here only to verify that the property is
  // reset upon processing a RAD line.
  AtomVector atoms{
      Atom({.atomic_num = 6, .props = {.chg = 1, .rad = 2}}),
      Atom({.atomic_num = 6, .props = {.chg = -1, .rad = 2}}),
      Atom({.atomic_num = 6, .props = {.chg = -1, .rad = 3}}),
  };

  REQUIRE(atoms.at(0).props().chg == 1);
  REQUIRE(atoms.at(0).props().rad == 2);
  REQUIRE(atoms.at(1).props().chg == -1);
  REQUIRE(atoms.at(1).props().rad == 2);
  REQUIRE(atoms.at(2).props().chg == -1);
  REQUIRE(atoms.at(2).props().rad == 3);

  BondVector bonds;

  V2000PropBlockReader pb_reader(line_reader, atoms, bonds);

  const auto read_result = pb_reader.read();
  REQUIRE(read_result.is_ok());

  REQUIRE(atoms.at(0).props().chg == 0);
  REQUIRE(atoms.at(0).props().rad == 3);
  REQUIRE(atoms.at(1).props().chg == 0);
  REQUIRE(atoms.at(1).props().rad == 1);
  REQUIRE(atoms.at(2).props().chg == 0);
  REQUIRE(atoms.at(2).props().rad == 0);
}

} // namespace
} // namespace mesaac::mol::internal
