//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>

namespace mesaac {
// Get information about atom radii, given element ID.
// Returns element atom radius, Angstroms
double getRadius(int atomicNumber);
// Get atom radius given its atom symbol:
double getSymbolRadius(const std::string &atomicSymbol);

// XXX FIX THIS Poor separation of concerns
// Get atomic number given atom symbol
unsigned char getAtomicNum(const std::string &atomicSymbol);
std::string getSymbol(int atomicNumber);

} // namespace mesaac
