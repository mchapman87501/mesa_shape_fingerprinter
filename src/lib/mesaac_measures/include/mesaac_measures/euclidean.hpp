//  OBS:  Euclidean is a distance measure.
//  This module provides a similarity measure, so technically it's providing
//  1 - Euclidean

#pragma once

#include "mesaac_measures/measures_base.hpp"

namespace mesaac::measures {
class Euclidean : public MeasuresBase {
public:
  std::string name() const override { return "Euclidean"; }
  float similarity(const shape_defs::BitVector &v1,
                   const shape_defs::BitVector &v2) const override;
};
} // namespace mesaac::measures
