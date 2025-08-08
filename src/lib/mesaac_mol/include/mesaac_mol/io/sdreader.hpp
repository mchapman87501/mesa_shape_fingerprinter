//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol/mol.hpp"
#include "mesaac_mol/result.hpp"
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>

namespace mesaac::mol {

class SDReaderImpl;

using BoolResult = Result<bool>;
using MolResult = Result<Mol>;
/**
 * @brief SDReader reads Mol instances from an SD file.
 */
struct SDReader {
  /**
   * @brief Create an SD file reader.
   * @param inf stream from which to read SD structures
   * @param pathname if given, the pathname from which `inf` is reading
   */
  SDReader(std::istream &inf, const std::string &pathname = "(input stream)");

  ~SDReader();

  /**
   * @brief Skip the next molecule.
   * @return true if the reader was able to skip ahead, else an error msg
   */
  BoolResult skip();

  /**
   * @brief Read the next molecule/structure.
   * @return a Mol, or an error msg
   */
  MolResult read();

  /**
   * @brief Find out whether the reader has reached the end of its input.
   * @return true if the reader has nothing more to read
   */
  bool eof() const;

private:
  std::unique_ptr<SDReaderImpl> m_impl;
  SDReader(const SDReader &src);
  SDReader &operator=(const SDReader &src);
};
} // namespace mesaac::mol
