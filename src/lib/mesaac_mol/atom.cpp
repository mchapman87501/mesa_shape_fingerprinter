//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/atom.hpp"
#include "mesaac_mol/element_info.hpp"

using namespace std;

namespace mesaac {
namespace mol {

void Atom::atomic_num(unsigned int new_value) { m_atomic_num = new_value; }

void Atom::x(float new_value) { m_x = new_value; }

void Atom::y(float new_value) { m_y = new_value; }

void Atom::z(float new_value) { m_z = new_value; }

void Atom::optional_cols(const string &new_value) {
  m_optional_cols = new_value;
}

string Atom::symbol() const { return get_symbol(m_atomic_num); }

float Atom::radius() const { return get_radius(m_atomic_num); }

bool Atom::is_hydrogen() const {
  bool result = (1 == m_atomic_num);
  return result;
}
} // namespace mol
} // namespace mesaac
