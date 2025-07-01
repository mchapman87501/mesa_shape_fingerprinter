// Unit test for mol::Mol
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include "mesaac_mol/mol.hpp"

using namespace std;

namespace mesaac::mol {
namespace {
string header_block(Mol &m) {
  ostringstream outs;
  outs << m.name() << "<br/>" << m.metadata() << "<br/>" << m.comments();
  return outs.str();
}

TEST_CASE("mesaac::mol::Mol", "[mesaac]") {

  SECTION("Basic tests") {

    Mol m1;
    REQUIRE(header_block(m1) == "<br/><br/>");
    Mol m2({.name = "A molecule", .metadata = "ill-formed", .comments = "t&t"});
    REQUIRE(header_block(m2) == ("A molecule<br/>ill-formed<br/>t&t"));
  } // namespace mesaac

  SECTION("Atoms and Bonds") {
    AtomVector atoms;
    const unsigned int num_atoms = 10;
    unsigned int i;
    for (i = 0; i < num_atoms; i++) {
      Atom atom({.atomic_num = i, .pos = {(float)i, 0.0f, 0.0f}});
      atoms.emplace_back(atom);
    }
    Mol mol({.atoms = atoms});
    REQUIRE(mol.num_atoms() == num_atoms);
    REQUIRE(mol.num_heavy_atoms() == num_atoms - 1);

    unsigned int visited = 0;
    for (const auto &atom : mol.atoms()) {
      REQUIRE(atom.atomic_num() == visited);
      REQUIRE(atom.x() == visited);
      REQUIRE(atom.y() == 0.0f);
      REQUIRE(atom.z() == 0.0f);
      visited++;
    }
    REQUIRE(visited == num_atoms);

    Bond b_orig({1, 2, BondType::bt_aromatic, BondStereo::bs_cis_trans_double,
                 "xxxrrrccc"});
    Mol mol2({.bonds = {b_orig}});
    visited = 0;
    for (const auto &bond : mol2.bonds()) {
      REQUIRE(bond.a0() == 1U);
      REQUIRE(bond.a1() == 2U);
      REQUIRE(bond.type() == BondType::bt_aromatic);
      REQUIRE(bond.stereo() == BondStereo::bs_cis_trans_double);
      REQUIRE(bond.optional_cols() == "xxxrrrccc");
      visited++;
    }
    REQUIRE(1u == visited);
  }

  SECTION("Properties") {
    Mol mol;
    REQUIRE(mol.properties_block() == "");

    string pb("M  1\nM  2\nM  END\n");
    Mol mol2({.properties_block = pb});
    REQUIRE(mol2.properties_block() == pb);
  }

  SECTION("Tags") {
    Mol mol;
    REQUIRE(mol.tags().empty());

    SDTagMap tags;
    tags.add("t1", 1.0);
    tags.add("t2", 2.0);
    const string first_value("3 and something");
    tags.add("t3", first_value);

    Mol mol2({.tags = tags});
    REQUIRE(mol2.tags().size() == 3);

    // XXX FIX THIS:  you can't retrieve a tag using the
    // same syntax as you used to set it...
    SDTagMap::const_iterator it = mol2.tags().find(">  <t3>");
    REQUIRE(it != mol2.tags().end());
    REQUIRE(it->second == first_value);

    // XXX FIX THIS - this belongs in a test_sd_tag_map TEST_CASE.
    // Mixing of concerns: verify SDTagMap uses "last one wins"
    // when updating content.
    tags.add("t3", 42.0);
    it = tags.find(">  <t3>");
    REQUIRE(it != tags.end());
    istringstream ins(it->second);
    double v;
    ins >> v;
    REQUIRE(v == 42.0);
  }
}

} // namespace
} // namespace mesaac::mol