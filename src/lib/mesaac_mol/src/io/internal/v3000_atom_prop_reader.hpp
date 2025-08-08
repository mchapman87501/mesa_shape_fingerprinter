#pragma once

#include "mesaac_mol/atom_props.hpp"
#include "mesaac_mol/result.hpp"
#include <sstream>
#include <string>

namespace mesaac::mol::internal {

struct V3000AtomPropReader {
  using Result = mesaac::mol::Result<AtomProps>;

  /**
   * @brief Read V3000 atom properties from a stream
   * @param ins stream initially positioned just after the aamap of an atom line
   * @return AtomProps read from ins, or an error message
   */
  Result read(std::istream &ins);
};

} // namespace mesaac::mol::internal
