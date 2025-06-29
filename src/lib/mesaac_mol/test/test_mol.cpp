// Unit test for mol::Mol
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include "mesaac_mol/mol.hpp"

using namespace std;

namespace mesaac {

namespace mol {
namespace {
string header_block(Mol &m) {
  ostringstream outs;
  outs << m.name() << "<br/>" << m.metadata() << "<br/>" << m.comments();
  return outs.str();
}
} // namespace

TEST_CASE("mesaac::mol::Mol", "[mesaac]") {

  SECTION("Basic tests") {

    Mol m1;
    REQUIRE(header_block(m1) == "<br/><br/>");
    m1.name("A molecule");
    m1.metadata("ill-formed");
    m1.comments("t&t");
    REQUIRE(header_block(m1) == ("A molecule<br/>ill-formed<br/>t&t"));
    m1.clear();
    REQUIRE(header_block(m1) == "<br/><br/>");
  } // namespace mesaac

  SECTION("Atoms and Bonds") {
    Mol mol;
    const unsigned int C_NumAtoms = 10;
    unsigned int i;
    for (i = 0; i < C_NumAtoms; i++) {
      Atom atom(i, {(float)i, 0.0f, 0.0f});
      mol.add_atom(atom);
    }
    REQUIRE(mol.num_atoms() == C_NumAtoms);
    REQUIRE(mol.num_heavy_atoms() == C_NumAtoms - 1);

    unsigned int visited = 0;
    for (const auto &atom : mol.atoms()) {
      REQUIRE(atom.atomic_num() == visited);
      REQUIRE(atom.x() == visited);
      REQUIRE(atom.y() == 0.0f);
      REQUIRE(atom.z() == 0.0f);
      visited++;
    }
    REQUIRE(visited == C_NumAtoms);

    Bond b_orig;
    b_orig.a0(1); // Bond numbers are one-based.
    b_orig.a1(2);
    b_orig.type(Bond::BTE_AROMATIC);
    b_orig.stereo(Bond::BSE_CIS_TRANS_DOUBLE);
    b_orig.optional_cols("xxxrrrccc");
    mol.add_bond(b_orig);
    visited = 0;
    for (const auto &bond : mol.bonds()) {
      REQUIRE(bond.a0() == 1U);
      REQUIRE(bond.a1() == 2U);
      REQUIRE(bond.type() == Bond::BTE_AROMATIC);
      REQUIRE(bond.stereo() == Bond::BSE_CIS_TRANS_DOUBLE);
      REQUIRE(bond.optional_cols() == "xxxrrrccc");
      visited++;
    }
    REQUIRE(1u == visited);

    mol.clear();
    REQUIRE(mol.num_atoms() == 0U);
    REQUIRE(mol.bonds().begin() == mol.bonds().end());
  }

  SECTION("Properties") {
    Mol mol;

    REQUIRE(mol.properties_block() == "");
    string pb("M  1\nM  2\nM  END\n");
    mol.properties_block(pb);
    REQUIRE(mol.properties_block() == pb);
    mol.clear();
    REQUIRE(mol.properties_block() == "");
  }

  SECTION("Tags") {
    Mol mol;
    REQUIRE(mol.tags().empty());
    mol.add_tag("t1", 1.0);
    mol.add_tag("t2", 2.0);
    REQUIRE(mol.tags().size() == 2);

    const string FirstValue("3 and something");
    mol.add_tag("t3", FirstValue);
    REQUIRE(mol.tags().size() == 3);

    // XXX FIX THIS:  you can't retrieve a tag using the
    // same syntax as you used to set it...
    SDTagMap::const_iterator it = mol.tags().find(">  <t3>");
    REQUIRE(it != mol.tags().end());
    REQUIRE(it->second == FirstValue);

    // Duplicate
    mol.add_tag("t3", 42.0);
    it = mol.tags().find(">  <t3>");
    REQUIRE(it != mol.tags().end());
    istringstream ins(it->second);
    double v;
    ins >> v;
    REQUIRE(v == 42.0);
  }
} // TEST_CASE
} // namespace mol
} // namespace mesaac