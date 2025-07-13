// Program Asym
// Tversky subclass methods.

#include "mesaac_measures/tversky.hpp"
#include <iostream>

using namespace std;

namespace mesaac::measures {

Tversky::Tversky(float analpha) {
  if ((analpha > 2.0) || (analpha < 0.0)) {
    cerr << "Tversky alpha is not in the range 0 to 2 -- hope that is ok."
         << endl;
  }
  alpha = analpha;
  beta = 2.0 - alpha;
}

float Tversky::similarity(const shape_defs::BitVector &v1,
                          const shape_defs::BitVector &v2) const {
  float a = (v1 & v2).count();
  float b = (v1 & ~v2).count();
  float c = (~v1 & v2).count();

  float result = 1.0; // if denom is zero, v1 and v2 must be zero, and ==.
  float denom = (a + alpha * b + beta * c);
  if (denom) {
    result = a / denom;
  }
  return result;
}

} // namespace mesaac::measures