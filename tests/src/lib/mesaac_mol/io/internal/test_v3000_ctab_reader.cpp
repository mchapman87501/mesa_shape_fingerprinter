#include <catch2/catch_test_macros.hpp>

#include "ctab.hpp"
#include "mol_header_block.hpp"
#include "v3000_ctab_reader.hpp"

#include <sstream>

namespace mesaac::mol::internal {
namespace {
TEST_CASE("mesaac::mol::internal::V3000CTabReader - no optional blocks",
          "[mesaac]") {
  // This example is derived from structures in the "SDF file MOL V3000
  // records" download from DrugCentral 2023 - https://drugcentral.org/download
  std::istringstream ins(R"LINES(levobupivacaine fragment
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 5 4 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V3000CTabReader ctab_reader(reader);

  const auto ctab_result = ctab_reader.read(block);
  REQUIRE(ctab_result.is_ok());

  const auto actual = ctab_result.value();
  REQUIRE(actual.atoms.size() == 5);
  REQUIRE(actual.bonds.size() == 4);
  REQUIRE(actual.raw_properties_block == "M  V30 END CTAB\n");
  REQUIRE(actual.post_ctab_block == "M  END\n");
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - atom properties",
          "[mesaac]") {
  std::istringstream ins(R"LINES(atom props
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 5 4 0 0 1
M  V30 BEGIN ATOM
M  V30 5 C 0 0 0 0 EXACHG=1
M  V30 4 C 0 0 0 0 CLASS=foo
M  V30 3 C 0 0 0 0 SEQID=4
M  V30 2 C 0 0 0 0 ATTCHORD=(4 8 Al 2 Br)
M  V30 1 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  V3000CTabReader ctab_reader(reader);

  const auto ctab_result = ctab_reader.read(block);
  REQUIRE(ctab_result.is_ok());

  const auto actual = ctab_result.value();
  REQUIRE(actual.atoms.size() == 5);

  const auto &atoms = actual.atoms;
  for (size_t i = 0; i < atoms.size(); ++i) {
    REQUIRE(atoms.at(i).props().index == 5 - i);
  }
  // Spot-check that some properties were assigned.
  REQUIRE(atoms[0].props().exachg == 1);
  REQUIRE(atoms[1].props().cta_class == "foo");
  REQUIRE(atoms[2].props().seqid == 4);
  REQUIRE(atoms[3].props().attchord == "(4 8 Al 2 Br)");
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - no bonds", "[mesaac]") {
  std::istringstream ins(R"LINES(no bonds
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 5 0 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  const auto ctab_result = ctab_reader.read(block);
  REQUIRE(ctab_result.is_ok());

  const auto actual = ctab_result.value();
  REQUIRE(actual.atoms.size() == 5);
  REQUIRE(actual.bonds.size() == 0);
  REQUIRE(actual.raw_properties_block == "M  V30 END CTAB\n");
  REQUIRE(actual.post_ctab_block == "M  END\n");
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - with optional CTAB blocks",
          "[mesaac]") {

  // This is derived from the Biovia CTfile Formats document,
  // documentation for the Sgroup block.  It is nonsense; I hope it
  // is syntactically valid.  (I have my doubts about that because it
  // contains obvious continuation lines that are not terminated with '-'.)
  std::istringstream ins(R"LINES(levobupivacaine fragment
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 5 4 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 BEGIN SGROUP
M  V30 1 SRU 5 ATOMS=(2 5 6)  XBONDS=(2 5 6) BRKXYZ=(9 -0.6103 1.2969 0 -
M  V30 -0.6103 0.171  0 0 0 0)
M  V30 END SGROUP
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  const auto actual_result = ctab_reader.read(block);
  REQUIRE(actual_result.is_ok());

  const auto actual = actual_result.value();
  REQUIRE(actual.atoms.size() == 5);
  REQUIRE(actual.bonds.size() == 4);
  REQUIRE(actual.raw_properties_block.starts_with("M  V30 BEGIN SGROUP\n"));
  REQUIRE(actual.raw_properties_block.ends_with(
      "M  V30 END SGROUP\nM  V30 END CTAB\n"));
  REQUIRE(actual.post_ctab_block == "M  END\n");
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - missing V30 prefix",
          "[mesaac]") {
  std::istringstream ins(R"LINES(missing V30 prefix
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 5 4 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  const auto ctab_result = ctab_reader.read(block);
  REQUIRE(!ctab_result.is_ok());
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - high atom count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(high atom count
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 10 4 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  REQUIRE(!ctab_reader.read(block).is_ok());
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - low atom count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(low atom count
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 4 4 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  REQUIRE(!ctab_reader.read(block).is_ok());
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - high bond count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(high bond count
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 5 14 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  REQUIRE(!ctab_reader.read(block).is_ok());
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - low bond count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(low bond count
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 5 2 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  REQUIRE(!ctab_reader.read(block).is_ok());
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - negative atom count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(negative atom count
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS -5 2 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  REQUIRE(!ctab_reader.read(block).is_ok());
}

TEST_CASE("mesaac::mol::internal::V3000CTabReader - negative bond count",
          "[mesaac]") {
  std::istringstream ins(R"LINES(negative bond count
  -INDIGO-06102310542D

  0  0  0     0  0            999 V3000
M  V30 BEGIN CTAB
M  V30 COUNTS 5 -42 0 0 1
M  V30 BEGIN ATOM
M  V30 1 C 0 0 0 0
M  V30 2 C 0 0 0 0
M  V30 3 C 0 0 0 0
M  V30 4 C 0 0 0 0
M  V30 5 N 0 0 0 0
M  V30 END ATOM
M  V30 BEGIN BOND
M  V30 1 1 1 2
M  V30 2 1 2 3
M  V30 3 1 3 4
M  V30 4 1 4 5
M  V30 END BOND
M  V30 END CTAB
M  END)LINES");

  LineReader reader(ins, "<from a string>");
  V3000CTabReader ctab_reader(reader);

  const auto header_result = MolHeaderBlock::read(reader);
  REQUIRE(header_result.is_ok());
  MolHeaderBlock block = header_result.value();

  REQUIRE(!ctab_reader.read(block).is_ok());
}
} // namespace
} // namespace mesaac::mol::internal
