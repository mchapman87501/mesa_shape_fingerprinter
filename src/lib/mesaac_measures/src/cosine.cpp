// Program Asym
// Cosine subclass methods.

#include "mesaac_measures/cosine.hpp"
#include <cmath>

namespace mesaac::measures {

float Cosine::similarity(const shape_defs::BitVector &v1,
                         const shape_defs::BitVector &v2) const {
  float result = 0.0;
  float a = v1.count();
  float b = v2.count();
  float c = (v1 & v2).count();
  float denom = sqrt(a * b);
  if (denom > 0) {
    result = c / denom;
  }
  return result;
}

} // namespace mesaac::measures