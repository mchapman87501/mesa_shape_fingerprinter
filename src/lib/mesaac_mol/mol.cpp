//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/mol.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

namespace mesaac {
namespace mol {

void Mol::clear() {
  m_name = "";
  m_metadata = "";
  m_comments = "";
  m_atoms.clear();
  m_bonds.clear();
  m_tags.clear();
  m_properties_block = "";
}

void Mol::name(const string &new_value) { m_name = new_value; }

void Mol::metadata(const string &new_value) { m_metadata = new_value; }

void Mol::comments(const string &new_value) { m_comments = new_value; }

void Mol::add_atom(Atom &atom) { m_atoms.push_back(atom); }

void Mol::add_bond(Bond &bond) { m_bonds.push_back(bond); }

void Mol::properties_block(const string &new_value) {
  m_properties_block = new_value;
}

void Mol::add_unparsed_tag(const string &tag_line, const string &value) {
  if (m_tags.find(tag_line) != m_tags.end()) {
    // TODO:  Throw exception
    cerr << "Warning: molecule already has tag '" << tag_line
         << "'.  Overwriting with new value." << endl;
  }
  m_tags[tag_line] = value;
}

void Mol::add_tag(const string &name, const string &value) {
  string tag = ">  <" + name + ">";
  add_unparsed_tag(tag, value);
}

unsigned int Mol::num_atoms() { return m_atoms.size(); }

unsigned int Mol::num_heavy_atoms() {
  return std::ranges::count_if(
      m_atoms, [](const Atom &atom) { return !atom.is_hydrogen(); });
}

// Based on a layman's reading of MDL ctfile spec:
// Returns 2 if all z coordinates are zero, 3 otherwise.
unsigned int Mol::dimensionality() {
  // TODO:  Cache this value, invalidating whenever atoms are added or
  // cleared.
  for (const Atom &a : m_atoms) {
    if (a.z() != 0) {
      return 3;
    }
  }
  return 2;
}
} // namespace mol
} // namespace mesaac
