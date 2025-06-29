//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/atom.hpp"
#include "mesaac_mol/element_info.hpp"

using namespace std;

namespace mesaac::mol {

string Atom::symbol() const { return get_symbol(m_atomic_num); }

float Atom::radius() const { return get_radius(m_atomic_num); }

bool Atom::is_hydrogen() const { return (1 == m_atomic_num); }
} // namespace mesaac::mol