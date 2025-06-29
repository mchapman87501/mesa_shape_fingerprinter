//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>

namespace mesaac::mol {
// Get information about atom radii, given element ID.
// Returns element atom radius, Angstroms
double get_radius(int atomic_number);
// Get atom radius given its atom symbol:
double get_symbol_radius(const std::string &atomic_symbol);

// XXX FIX THIS Poor separation of concerns
// Get atomic number given atom symbol
unsigned char get_atomic_num(const std::string &atomic_symbol);
std::string get_symbol(int atomic_number);

} // namespace mesaac::mol
