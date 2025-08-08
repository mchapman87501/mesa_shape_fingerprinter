//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>

namespace mesaac::mol {
// Get information about atom radii, given element ID.
// Returns element atom radius, Ångstroms
float get_radius(int atomic_number);
// Get atom radius given its atom symbol:
float get_symbol_radius(const std::string &atomic_symbol);

// XXX FIX THIS Poor separation of concerns
// Get atomic number given atom symbol
unsigned char get_atomic_num(const std::string &atomic_symbol);
std::string get_symbol(int atomic_number);

/**
 * @brief Get the "natural abundance" atomic weight for an atomic number.
 * @param atomic_number atomic number of an element
 * @return the atomic weight
 */
float get_atomic_mass(int atomic_number);

} // namespace mesaac::mol
