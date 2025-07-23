#pragma once

#include "mesaac_mol/mol.hpp"
#include <memory>
#include <string>

namespace mesaac::mol {

class ISDReader;

/**
 * @brief PathSDReader is for Swift-C++ interop.
 * @details Swift wants to treat C++ classes as Copyable value types.  But
 * the istream exposed in the interface of mesaac::mol::SDReader is not
 * copyable.  Hence this wrapper.
 */
struct PathSDReader {
  PathSDReader(const std::string &sd_file_content, const std::string &filename);
  PathSDReader(const std::string &pathname);
  PathSDReader(const PathSDReader &src);
  PathSDReader(PathSDReader &&src);
  PathSDReader &operator=(const PathSDReader &src);
  PathSDReader &operator=(PathSDReader &&src);

  bool read(Mol &mol);
  bool skip();

private:
  std::shared_ptr<ISDReader> m_impl;
};
} // namespace mesaac::mol