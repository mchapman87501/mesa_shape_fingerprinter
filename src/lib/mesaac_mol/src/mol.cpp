//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/mol.hpp"

#include <algorithm>
#include <iostream>
#include <sstream>

using namespace std;

namespace mesaac::mol {

unsigned int Mol::num_heavy_atoms() const {
  return std::ranges::count_if(
      m_atoms, [](const Atom &atom) { return !atom.is_hydrogen(); });
}

// Based on a layman's reading of MDL ctfile spec:
// Returns 2 if all z coordinates are zero, 3 otherwise.
unsigned int Mol::dimensionality() const {
  // TODO:  Cache this value, invalidating whenever atoms are added or
  // cleared.
  for (const Atom &a : m_atoms) {
    if (a.pos().z() != 0) {
      return 3;
    }
  }
  return 2;
}
} // namespace mesaac::mol
