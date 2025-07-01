// Unit test for mol::SDWriter
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include <fstream>

#include "mesaac_mol/io.hpp"

using namespace std;

namespace mesaac::mol {
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

string file_content(string &pathname) {
  ostringstream outs;
  const unsigned int length = 8192;
  char buffer[length];

  ifstream inf(pathname.c_str());
  while (inf) {
    inf.read(buffer, length - 1);
    buffer[inf.gcount()] = '\0';
    outs << buffer;
  }

  return outs.str();
}

const string test_sdf_path(const string &rel_path) {
  const string test_data_dir(TEST_DATA_DIR);
  const string mesaac_mol_data_in_dir(test_data_dir + "/lib/mesaac_mol/in/");
  return mesaac_mol_data_in_dir + rel_path;
}

TEST_CASE("mesaac::mol::SDWriter", "[mesaac]") {

  SECTION("Dimensionality") {
    // Based on a layman's reading of MDL ctfile spec:
    // Dimensionality of a molecule should be 3 if it contains
    // any non-zero z coordinates, 2 otherwise.
    Atom atom1({.atomic_num = 1});
    Mol m1({.atoms = {atom1}});
    REQUIRE(m1.dimensionality() == 2u);

    Atom atom2({1, {1.0f, 0.0f, 0.0f}});
    Mol m2({.atoms = {atom1, atom2}});
    REQUIRE(m2.dimensionality() == 2u);

    Atom atom3({1, {1.0f, 1.0f, 0.0f}});
    Mol m3({.atoms = {atom1, atom2, atom3}});
    REQUIRE(m3.dimensionality() == 2u);

    Atom atom4({1, {1.0f, 1.0f, 1.0f}});
    Mol m4({.atoms = {atom1, atom2, atom3, atom4}});
    REQUIRE(m4.dimensionality() == 3u);
  }

  SECTION("Basic writing") {
    // OBS:  mol::SDWriter will write out its tags in by lexical order
    // of tag header.  This may not be the same as the input.
    // For simplified testing, use an input file whose tags are
    // already sorted lexically by tag header.
    string pathname(test_sdf_path("sorted_tags.sdf"));
    ifstream inf(pathname.c_str());
    SDReader reader(inf, pathname);

    ostringstream outs;
    SDWriter writer(outs);

    Mol m;
    while (reader.read(m)) {
      writer.write(m);
    }
    inf.close();

    // Output should be identical to input, except for the program data
    // lines.  Ignore the program data lines.
    istringstream actualf(outs.str());
    ifstream expectedf(pathname.c_str());
    unsigned int i_mol_line = 0, i = 0;
    ostringstream diffs;
    while (actualf && expectedf) {
      i++;
      i_mol_line++;

      string expected, actual;
      std::getline(actualf, actual);
      std::getline(expectedf, expected);
      if (i_mol_line != 2) {
        if (actual != expected) {
          diffs << "Line " << i << ": '" << expected << "' != '" << actual
                << "'" << endl;
        }
      }
      if (actual == "$$$$") {
        i_mol_line = 0;
      }
    }
    if (actualf) {
      diffs << "Actual output has more lines than expected." << endl;
    } else if (expectedf) {
      diffs << "Expected output has more lines than actual." << endl;
    }
    // This is a good way to test for success, but a lousy way
    // to report differences...
    if (diffs.str().size() > 0) {
      ofstream actf("actual_test_sdwriter.sdf");
      actf << outs.str();
      actf.close();

      FAIL(diffs.str());
    }
  }

  SECTION("Malformed empty tag") {
    // Some utilities produce invalid SD tags -- ones which have
    // a tag followed by just one empty line instead of two or more.
    // Ensure the writer can output these records correctly.
    string pathname(test_sdf_path("malformed_empty_tag_value.sdf"));
    ifstream inf(pathname.c_str());
    SDReader reader(inf, pathname);
    Mol m;
    int occurrences = 0;
    while (reader.read(m)) {
      ostringstream outs;
      SDWriter writer(outs);
      writer.write(m);

      string s(outs.str());
      ostringstream tag;
      tag << ">  <empty>" << endl << endl;
      if (string::npos != s.find(tag.str())) {
        // If the tag is present, verify that its empty
        // value is also present -- and fixed so it has an
        // extra blank line.
        ostringstream empty_tag;
        empty_tag << ">  <empty>" << endl
                  << endl
                  << endl
                  << ">  <non_empty>" << endl;
        if (string::npos == s.find(empty_tag.str())) {
          cerr << "Did not find expected empty tag in '" << s << "'." << endl;
        }
        REQUIRE(string::npos != s.find(empty_tag.str()));
        occurrences++;
      }
    }
    REQUIRE(occurrences == 2);
    inf.close();
  }

  SECTION("Transform and tagging") {
    // To be useful in align_monte, mol::SDWriter must be able to handle
    // changes in atom coordinates and addition of tags.

    string pathname(test_sdf_path("one_structure.sdf"));
    ifstream inf(pathname.c_str());
    SDReader reader(inf, pathname);
    Mol m_in;
    REQUIRE(reader.read(m_in));
    inf.close();

    // Take it on faith that the hydrogens are not all superposed
    // at (-100,-100,-100) in the input file.
    unsigned int hcount = 0;
    for (auto &atom : m_in.mutable_atoms()) {
      if (atom.is_hydrogen()) {
        atom.set_pos({-100.0f, -100.0f, -100.0f});
        hcount++;
      }
    }

    SDTagMap tags;
    tags.add("test_sdwriter.hcount", hcount);
    tags.add("test_sdwriter.blank_terminated", "foo\n\n");
    tags.add("test_sdwriter.not_blank_termed", "foo");
    tags.add("test_sdwriter.multi_blank_termed", "foo    \n\n\n\n");

    Mol m_tagged({.atoms = m_in.atoms(), .tags = tags});

    ostringstream outs;
    SDWriter writer(outs);
    writer.write(m_tagged);
    unsigned int written_hcount = 0;
    string s(outs.str());
    const string pattern(" -100.0000 -100.0000 -100.0000 H ");
    size_t i_start = 0;
    while (true) {
      i_start = s.find(pattern, i_start);
      if (i_start == string::npos) {
        break;
      }
      i_start += pattern.size();
      written_hcount++;
    }
    REQUIRE(written_hcount == hcount);

    {
      ostringstream tags;
      tags << ">  <test_sdwriter.hcount>" << endl << hcount << endl << endl;
      REQUIRE(string::npos != s.find(tags.str()));
    }
    {
      ostringstream tags;
      tags << ">  <test_sdwriter.blank_terminated>" << endl
           << "foo" << endl
           << endl;
      REQUIRE(string::npos != s.find(tags.str()));
    }
    {
      ostringstream tags;
      tags << ">  <test_sdwriter.not_blank_termed>" << endl
           << "foo" << endl
           << endl;
      REQUIRE(string::npos != s.find(tags.str()));
    }
    {
      ostringstream tags;
      // Weak!  Doesn't test for too many blank lines.
      tags << ">  <test_sdwriter.multi_blank_termed>" << endl
           << "foo" << endl
           << endl;
      REQUIRE(string::npos != s.find(tags.str()));
    }
    cout << "TODO:  Test for lines containing only whitespace." << endl;
  }
}
} // namespace
} // namespace mesaac::mol
