//  OBS:  Euclidean is a distance measure.
//  This module provides a similarity measure, so technically it's providing
//  1 - Euclidean

#include "mesaac_measures/euclidean.hpp"
#include <cmath>

namespace mesaac::measures {

float Euclidean::similarity(const shape_defs::BitVector &v1,
                            const shape_defs::BitVector &v2) const {
  float a = (v1 ^ v2).count();
  float distance = sqrt(a / v1.size());
  return 1.0 - distance;
}

} // namespace mesaac::measures
