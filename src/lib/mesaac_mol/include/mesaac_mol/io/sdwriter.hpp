//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol/mol.hpp"
#include <iostream>
#include <sstream>

namespace mesaac::mol {
/// @brief SDWriter writes molecules to an output stream, in SD format.
class SDWriter {
public:
  /// @brief Create a new instance.
  /// @param outf stream to which to write molecules.
  SDWriter(std::ostream &outf);

  /// @brief Write a molecule, in SD format.
  /// @details Writes molecule `mol` to the output stream of this instance.
  /// @param mol the molecule to write
  /// @return true if `mol` was written successfully, false otherwise
  bool write(Mol &mol);

protected:
  std::ostream &m_outf;

private:
  SDWriter(const SDWriter &src);
  SDWriter(SDWriter &&src);
  SDWriter &operator=(const SDWriter &src);
  SDWriter &operator=(const SDWriter &&src);
};
} // namespace mesaac::mol
