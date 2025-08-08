#pragma once

#include <memory>
#include <string>

#include "mesaac_mol/mol.hpp"
#include "mesaac_mol/result.hpp"

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

  Result<Mol> read();
  Result<bool> skip();
  bool eof() const;
  std::string pathname() const;

private:
  std::shared_ptr<ISDReader> m_impl;
};
} // namespace mesaac::mol