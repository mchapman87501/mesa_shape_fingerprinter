// Program Measures
// Measures class methods

#include "mesaac_measures/measures_base.hpp"

namespace mesaac::measures {

float MeasuresBase::similarity(const shape_defs::BitVector &v1,
                               const shape_defs::BitVector &v2) const {
  float result = 0.0;
  float b = (v1 | v2).count();
  if (0 != b) {
    float a = (v1 & v2).count();
    result = a / b;
  }
  return result;
}

} // namespace mesaac::measures
