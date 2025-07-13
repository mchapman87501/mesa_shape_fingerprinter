// Program Asym
// Hamann subclass methods.

#include "mesaac_measures/hamann.hpp"
namespace mesaac::measures {

float Hamann::similarity(const shape_defs::BitVector &v1,
                         const shape_defs::BitVector &v2) const {

  // This definition is from https://www.stata.com/manuals/mvmeasure_option.pdf
  // a - the number of bits which are set in both v1 and v2
  // b - the number of bits which are set in v1 and unset in v2
  // c - the number of bits which are unset in v1 and set in v2
  // d - the number of bits which are unset in both v1 and v2
  // In this definition, the range of values is from -1 (perfectly dissimilar)
  // to +1 (perfectly similar).
  const float a = (v1 & v2).count();
  const float b = (v1 & ~v2).count();
  const float c = (~v1 & v2).count();
  const float d = (~v1 & ~v2).count();
  const float result = ((a + d) - (b + c)) / (a + b + c + d);

  return result;
}

} // namespace mesaac::measures