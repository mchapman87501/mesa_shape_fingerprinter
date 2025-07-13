//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_measures/bub.hpp"
#include <cmath>

using namespace std;

namespace mesaac::measures {

float BUB::similarity(const shape_defs::BitVector &v1,
                      const shape_defs::BitVector &v2) const {
  float result = 0.0;
  shape_defs::BitVector both(v1 & v2);
  shape_defs::BitVector either(v1 | v2);
  shape_defs::BitVector neither(~either);
  unsigned int a = both.count();
  unsigned int d = neither.count();

  float s = ::sqrt(a * d);
  float denom = s + either.count();
  if (denom > 0) {
    result = (s + a) / denom;
  }
  return result;
}

} // namespace mesaac::measures
