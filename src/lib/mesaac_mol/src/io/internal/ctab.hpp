#pragma once
//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include <string>

#include "mesaac_mol/atom.hpp"
#include "mesaac_mol/bond.hpp"

namespace mesaac::mol::internal {

struct CTab {
  std::string name;
  std::string metadata;
  std::string comments;
  std::string counts_line;
  AtomVector atoms;
  BondVector bonds;
  std::string raw_properties_block;
  std::string post_ctab_block;
};

} // namespace mesaac::mol::internal
