//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol/mol.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace mesaac::mol {

class SDReaderImpl;

/**
 * @brief SDReader reads Mol instances from an SD file.
 */
class SDReader {
public:
  /**
   * @brief Create an SD file reader.
   * @param inf stream from which to read SD structures
   * @param pathname if given, the pathname from which `inf` is reading
   */
  SDReader(std::istream &inf, const std::string &pathname = "(input stream)");

  ~SDReader();

  /**
   * @brief Skip the next molecule.
   * @return true if the reader was able to skip ahead
   */
  bool skip();

  /**
   * @brief Read the next molecule/structure.
   * @param mol on successful return, the next read molecule
   * @return whether or not a molecule could be read into `mol`
   *
   * @note If the return value is `false`, then `mol` will be an empty molecule.
   */
  bool read(Mol &mol);

private:
  std::unique_ptr<SDReaderImpl> m_impl;
  SDReader(const SDReader &src);
  SDReader &operator=(const SDReader &src);
};
} // namespace mesaac::mol
