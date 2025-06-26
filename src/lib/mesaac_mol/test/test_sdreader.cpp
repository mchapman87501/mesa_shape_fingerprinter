// Unit test for mol::SDReader.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

// Confirm that the top-level include really pulls in all mesa_mol
// headers:
#include "mesaac_mol.hpp"

using namespace std;

namespace mesaac {

namespace {
string strdiff_summary(const string &s1, const string &s2) {
  unsigned int i;
  unsigned int i_max = s1.size();
  if (s2.size() > i_max) {
    i_max = s2.size();
  }
  ostringstream outf;
  for (i = 0; i != i_max; i++) {
    if (i >= s1.size()) {
      outf << "<";
    } else if (i >= s2.size()) {
      outf << ">";
    } else if (s1[i] != s2[i]) {
      outf << "X";
    } else {
      outf << ".";
    }
  }
  return outf.str();
}

class WhiteBoxMol : public mol::Mol {
public:
  unsigned int num_bonds() { return m_bonds.size(); }

  string tagstr() {
    ostringstream resultf;
    mol::SDTagMap::iterator i;
    // Maps are supposed to keep their keys in sorted order.
    for (const auto &entry : m_tags) {
      resultf << "'" << entry.first << "' = '" << entry.second << "'" << endl;
    }
    return resultf.str();
  }
};
} // namespace

TEST_CASE("mesaac::mol::sdreader", "[mesaac]") {
  // Test data for this test is in the same directory as this source
  // file.
  const string test_data_dir(TEST_DATA_DIR);
  const string mesaac_mol_data_in_dir(test_data_dir + "/lib/mesaac_mol/in/");

  SECTION("One structure") {
    string pathname(mesaac_mol_data_in_dir + "one_structure.sdf");

    ifstream inf(pathname.c_str());
    mol::SDReader reader(inf, pathname);
    unsigned int num_found = 0;
    WhiteBoxMol m;
    // O for C++0x and its static initializers.
    const char *expected_symbols[] = {
        "C", "N", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C",
        "C", "C", "C", "C", "F", "C", "S", "O", "O", "C", "H", "H", "H",
        "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H",
    };
    while (reader.read(m)) {
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

      const mol::Atom &atom(*m.atoms().begin());
      REQUIRE(atom.x() == 27.7051f);
      REQUIRE(atom.y() == 22.0403f);
      REQUIRE(atom.z() == 17.0243f);
      REQUIRE(atom.symbol() == string("C"));
      REQUIRE(atom.optional_cols() == string(" 0  0  0  0  0  0"));

      num_found++;
    }
    inf.close();
    REQUIRE(num_found == 1u);
  }

  SECTION("Multiple structures") {
    string pathname(mesaac_mol_data_in_dir + "cox2_3d.sd");
    // Spot-check some atom and bond counts.
    unsigned int mol_check_indices[] = {0, 10, 456, 466};
    const unsigned int num_to_check =
        sizeof(mol_check_indices) / sizeof(mol_check_indices[0]);

    unsigned int expected_num_atoms[] = {39, 45, 55, 36};
    unsigned int expected_num_bonds[] = {
        41,
        47,
        58,
        38,
    };

    ifstream inf(pathname.c_str());
    mol::SDReader reader(inf, pathname);

    unsigned int num_found = 0;
    unsigned int i_check = 0;
    unsigned int *check_index = mol_check_indices;
    WhiteBoxMol m;

    while (reader.read(m)) {
      while ((i_check < num_to_check) && (num_found > *check_index)) {
        i_check++;
        check_index++;
      }
      if ((i_check < num_to_check) && (num_found == *check_index)) {
        REQUIRE(m.num_atoms() == expected_num_atoms[i_check]);
        REQUIRE(m.num_bonds() == expected_num_bonds[i_check]);
        i_check++;
        check_index++;
      }
      num_found++;
    }
    REQUIRE(num_found == 467u);
  }

  SECTION("Properties block") {
    // Show we can read properties blocks.
    string pathname(mesaac_mol_data_in_dir + "property_blocks.sdf");
    ifstream inf(pathname.c_str());
    mol::SDReader reader(inf, pathname);

    WhiteBoxMol m;

    // Check the first and last molecules.
    REQUIRE(reader.read(m));
    const string exp_first_block("M  CHG  2   2   1  19  -1\nM  END\n");
    REQUIRE(m.properties_block() == exp_first_block);

    string last_properties_block(m.properties_block());
    while (reader.read(m)) {
      last_properties_block = m.properties_block();
    }

    const string exp_last_block("M  CHG  2  31   1  33  -1\nM  END\n");
    REQUIRE(last_properties_block == exp_last_block);
  }

  SECTION("Tags") {
    string pathname(mesaac_mol_data_in_dir + "property_blocks.sdf");
    ifstream inf(pathname.c_str());
    mol::SDReader reader(inf, pathname);

    WhiteBoxMol m;

    // Check the first and last molecules.
    REQUIRE(reader.read(m));
    const string exp_first("");
    if (exp_first != m.tagstr()) {
      cerr << "tag strings don't match:" << endl
           << "Diff    : " << strdiff_summary(exp_first, m.tagstr()) << endl;
    }
    REQUIRE(m.tagstr() == exp_first);

    string prev(m.tagstr());
    while (reader.read(m)) {
      prev = m.tagstr();
    }

    const string exp_last(
        "'>   (MD-0894)	<BOILING.POINT>	FROM ARCHIVES' = 'Yet another example, "
        "with extensive whitespace after the '> '.\n'\n'> (MD-0894)	"
        "<BOILING.POINT>	FROM ARCHIVES' = 'Yet another example.\n'\n'> "
        "<Attribute>' = '1\n'\n'> <ID>' = '644742\n'\n'> DT12	55' = 'Another "
        "sample\nwith multiline values.\n'\n'>55 (MD-08974)	"
        "<BOILING.POINT>	DT12' = 'This is a sample tag from the ctfile "
        "spec.\n'\n");
    if (exp_last != prev) {
      cerr << "tag strings don't match:" << endl
           << "Diff    : " << strdiff_summary(exp_last, prev) << endl;
    }
    REQUIRE(prev == exp_last);
  }

  SECTION("Garbage input :)") {
    // Try reading from a corrupt SD file, one in which newlines
    // have been smooshed into spaces.
    string pathname(mesaac_mol_data_in_dir + "corrupt.sdf");
    ifstream inf(pathname.c_str());
    mol::SDReader reader(inf, pathname);
    unsigned int num_found = 0;
    WhiteBoxMol m;
    while (reader.read(m)) {
      REQUIRE(m.num_atoms() > 0);
      num_found++;
    }
    inf.close();
    REQUIRE(num_found == 0u);
  }
}
} // namespace mesaac
