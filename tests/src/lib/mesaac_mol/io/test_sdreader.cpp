// Unit tests for mol::SDReader.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <fstream>
#include <vector>

#include "mesaac_mol/io/sdreader.hpp"
#include "mesaac_mol/mol.hpp"

namespace mesaac::mol {
namespace {
using namespace std;

void require_tag_str_match(const string &expected, const string &actual) {
  if (expected != actual) {
    const unsigned int i_max = max(expected.size(), actual.size());
    ostringstream outs;
    outs << "Tag strings don't match:" << endl << "Diff: ";
    for (unsigned int i = 0; i != i_max; i++) {
      if (i >= expected.size()) {
        outs << "<";
      } else if (i >= actual.size()) {
        outs << ">";
      } else {
        outs << ((expected[i] != actual[i]) ? "X" : ".");
      }
    }
    FAIL(outs.str());
  }
}

const std::filesystem::path test_sdf_path(const std::string &rel_path) {
  const std::filesystem::path test_data_dir(TEST_DATA_DIR);
  const std::filesystem::path sd_files_dir = test_data_dir / "sd_files";
  return sd_files_dir / rel_path;
}

string tagstr(const Mol &m) {
  ostringstream resultf;
  // Maps are supposed to keep their keys in sorted order.
  for (const auto &[key, value] : m.tags()) {
    resultf << "'" << key << "' = '" << value << "'" << endl;
  }
  return resultf.str();
}

unsigned int num_records(const std::filesystem::path &pathname) {
  ifstream inf(pathname);
  SDReader reader(inf, pathname);
  unsigned int result = 0;
  while (reader.read().is_ok()) {
    result += 1;
  }
  inf.close();
  return result;
}

TEST_CASE("mesaac::mol::SDReader - One structure", "[mesaac]") {
  std::filesystem::path pathname(test_sdf_path("one_structure.sdf"));

  ifstream inf(pathname);
  SDReader reader(inf, pathname);
  unsigned int num_found = 0;
  // O for C++0x and its static initializers.
  const char *expected_symbols[] = {
      "C", "N", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C",
      "C", "C", "C", "C", "F", "C", "S", "O", "O", "C", "H", "H", "H",
      "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H",
  };

  for (;;) {
    const auto result = reader.read();
    if (!result.is_ok()) {
      break;
    }
    const auto m = result.value();
    cout << "DEBUG: did read molecule." << endl;
    // First (and last) molecule should have the expected
    // properties.
    REQUIRE(m.num_atoms() == 39u);
    REQUIRE(m.num_heavy_atoms() == 23u);
    REQUIRE(m.num_bonds() == 41u);
    // No bounds check -- if it crashes, the test fails :)
    unsigned int i = 0;
    for (const auto &atom : m.atoms()) {
      REQUIRE(atom.symbol() == expected_symbols[i]);
      ++i;
    }

    // Verify expected info for first atom:
    // 27.7051   22.0403   17.0243 C   0  0  0  0  0  0

    const Atom &atom(*m.atoms().begin());
    REQUIRE(atom.pos().x() == 27.7051f);
    REQUIRE(atom.pos().y() == 22.0403f);
    REQUIRE(atom.pos().z() == 17.0243f);
    REQUIRE(atom.symbol() == string("C"));
    REQUIRE(atom.optional_cols() == string(" 0  0  0  0  0  0"));

    num_found++;
  }
  inf.close();
  REQUIRE(num_found == 1u);
}

TEST_CASE("mesaac::mol::SDReader - Atom list block ignored", "[mesaac]") {
  cerr << "To be written: demonstrate that V2000 atom list blocks are ignored."
       << endl;
}

TEST_CASE("mesaac::mol::SDReader - Multiple structures", "[mesaac]") {
  std::filesystem::path pathname(test_sdf_path("cox2_3d.sd"));
  // Spot-check some atom and bond counts.
  struct SpotCheck {
    const unsigned int index, num_atoms, num_bonds;

    bool has_expected_atom_count(const Mol &mol) const {
      return mol.num_atoms() == num_atoms;
    }

    bool has_expected_bond_count(const Mol &mol) const {
      return mol.num_bonds() == num_bonds;
    }
  };
  vector<SpotCheck> spot_checks{
      {0, 39, 41},
      {10, 45, 47},
      {456, 55, 58},
      {466, 36, 38},
  };

  ifstream inf(pathname);
  SDReader reader(inf, pathname);

  // Read all structures, spot-checking the numbers of atoms and bonds.
  auto curr_check = spot_checks.begin();
  unsigned int mol_index = 0;
  for (;;) {
    const auto result = reader.read();
    // Stop when no more structures can be read.
    if (!result.is_ok()) {
      break;
    }
    const auto mol = result.value();

    // If this is one of the spot-check mols, do the spot check.
    if (curr_check != spot_checks.end() && curr_check->index == mol_index) {
      REQUIRE(curr_check->has_expected_atom_count(mol));
      REQUIRE(curr_check->has_expected_bond_count(mol));
      curr_check++;
    }

    mol_index += 1;
  }
  REQUIRE(mol_index == 467u);
}

TEST_CASE("mesaac::mol::SDReader - Properties block", "[mesaac]") {
  // Show we can read properties blocks.
  // The block contents are not used by mesaac::mol.  They are preserved
  // and written verbatim when Mols are written - a claim that is not
  // tested here.
  std::filesystem::path pathname(test_sdf_path("property_blocks.sdf"));
  ifstream inf(pathname);
  SDReader reader(inf, pathname);

  // Check the first and last molecules.
  auto result = reader.read();
  if (!result.is_ok()) {
    FAIL(result.error());
  }

  // Properties are now being stored on atoms and bonds.
  // The properties block of a read molecule is unused.
  // The way to test is to check atom and bond properties.
  auto m = result.value();
  REQUIRE(m.atoms().at(1).props().chg == 1);
  REQUIRE(m.atoms().at(18).props().chg == -1);

  auto last_mol = m;
  for (;;) {
    const auto curr_result = reader.read();
    if (!curr_result.is_ok()) {
      break;
    }
    last_mol = curr_result.value();
  }

  REQUIRE(last_mol.atoms().at(30).props().chg == 1);
  REQUIRE(last_mol.atoms().at(32).props().chg == -1);
}

TEST_CASE("mesaac::mol::SDReader - Tags", "[mesaac]") {
  std::filesystem::path pathname(test_sdf_path("property_blocks.sdf"));
  ifstream inf(pathname);
  SDReader reader(inf, pathname);

  // Check the first and last molecules.

  const auto result = reader.read();
  REQUIRE(result.is_ok());

  auto m = result.value();
  const string exp_first("");
  const string actual_first = tagstr(m);
  require_tag_str_match(exp_first, actual_first);

  string prev(actual_first);
  for (;;) {
    const auto curr_result = reader.read();
    if (!curr_result.is_ok()) {
      break;
    }
    const auto m = curr_result.value();
    prev = tagstr(m);
  }

  const string exp_last(
      "'>   (MD-0894)	<BOILING.POINT>	FROM ARCHIVES' = 'Yet another example, "
      "with extensive whitespace after the '> '.\n'\n'> (MD-0894)	"
      "<BOILING.POINT>	FROM ARCHIVES' = 'Yet another example.\n'\n'> "
      "<Attribute>' = '1\n'\n'> <ID>' = '644742\n'\n'> DT12	55' = 'Another "
      "sample\nwith multiline values.\n'\n'>55 (MD-08974)	"
      "<BOILING.POINT>	DT12' = 'This is a sample tag from the ctfile "
      "spec.\n'\n");
  require_tag_str_match(exp_last, prev);
}

TEST_CASE("mesaac::mol::SDReader - Truncated counts line", "[mesaac]") {
  // Try reading from a corrupt SD file, one in which the
  // count line is shorter than it should be.
  // This should fail to read any atoms or bonds.
  // It should fail to read the one molecule in the file.
  std::filesystem::path pathname(test_sdf_path("truncated_count_line.sdf"));
  REQUIRE(num_records(pathname) == 0);
}

TEST_CASE("mesaac::mol::SDReader - Malformed atom counts", "[mesaac]") {
  // Try reading from a corrupt SD file, one in which the
  // count line has a malformed atom count.
  // This should fail to read any atoms at all.
  // It should also fail to read the one molecule in the SD file.
  std::filesystem::path pathname(test_sdf_path("malformed_atom_count.sdf"));
  REQUIRE(num_records(pathname) == 0);
}

TEST_CASE("mesaac::mol::SDReader - Malformed bond counts", "[mesaac]") {
  // Try reading from a corrupt SD file, one in which the
  // count line has a malformed bond count.
  std::filesystem::path pathname(test_sdf_path("malformed_bond_count.sdf"));
  REQUIRE(num_records(pathname) == 0);
}

TEST_CASE("mesaac::mol::SDReader - Garbage input", "[mesaac]") {
  // Try reading from a corrupt SD file, one in which newlines
  // have been smooshed into spaces.
  std::filesystem::path pathname(test_sdf_path("corrupt.sdf"));
  REQUIRE(num_records(pathname) == 0);
}

TEST_CASE("mesaac::mol::SDReader - V2000 no-structure", "[mesaac]") {
  // Ensure ability to read "no-structure" molfiles.
  const auto no_structure_sd = R"LINES(No structure

This is a no-structure as documented in CTfile Formats, ch. 3.
  0  0  0  0  0  0  0  0  0  0999 V2000
M  END
$$$$)LINES";
  string pathname("<no-structure - see CTfile Formats chapter 3.>");
  istringstream ins(no_structure_sd);

  std::vector<Mol> mols;
  std::vector<std::string> error_msgs;

  SDReader reader(ins, pathname);

  const auto read_result = reader.read();
  REQUIRE(read_result.is_ok());

  const auto mol = read_result.value();
  REQUIRE(mol.name() == "No structure");
  REQUIRE(mol.num_atoms() == 0);
  REQUIRE(mol.num_bonds() == 0);
}
} // namespace
} // namespace mesaac::mol
