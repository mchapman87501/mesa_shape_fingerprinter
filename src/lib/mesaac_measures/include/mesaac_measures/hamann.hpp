// Program Asym
// Hamann.h
// Hamann  measure sub-class

#pragma once

#include "mesaac_measures/measures_base.hpp"

namespace mesaac::measures {

class Hamann : public MeasuresBase {
public:
  std::string name() const override { return "Hamann"; }
  float similarity(const shape_defs::BitVector &v1,
                   const shape_defs::BitVector &v2) const override;
};
} // namespace mesaac::measures