//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol/mol.hpp"

#include <iostream>
#include <memory>
#include <sstream>
namespace mesaac::mol {
/// @brief SDWriter writes molecules to an output stream, in V2000 SD format.
class SDWriter {
public:
  using Ptr = std::shared_ptr<SDWriter>;

  /// @brief Create a new instance.
  /// @param outf stream to which to write molecules.
  SDWriter(std::ostream &outf);

  /// @brief Write a molecule, in SD format.
  /// @details Writes molecule `mol` to the output stream of this instance.
  /// @param mol the molecule to write
  /// @return true if `mol` was written successfully, false otherwise
  bool write(const Mol &mol);

private:
  std::ostream &m_outf;

  SDWriter(const SDWriter &src);
  SDWriter(SDWriter &&src);
  SDWriter &operator=(const SDWriter &src);
  SDWriter &operator=(const SDWriter &&src);

  bool write_atom(const Atom &atom) const;
  bool write_properties_block(const Mol &mol) const;
};
} // namespace mesaac::mol
