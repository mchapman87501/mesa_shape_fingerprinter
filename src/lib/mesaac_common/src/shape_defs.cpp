#include "mesaac_common/shape_defs.hpp"

namespace mesaac::shape_defs {
BitVector bit_vector_from_str(const std::string &strval) {
  return BitVector(strval);
}
} // namespace mesaac::shape_defs