//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_measures/measures_base.hpp"

namespace mesaac::measures {
/**
 * @brief Baroni-Urbani-Buser (BUB)
 */
class BUB : public MeasuresBase {
public:
  std::string name() const override { return "BUB"; }
  float similarity(const shape_defs::BitVector &v1,
                   const shape_defs::BitVector &v2) const override;
};
} // namespace mesaac::measures