// Program Asym
// Tversky.h
// Tversky measure sub-class

#pragma once

#include "mesaac_measures/measures_base.hpp"

namespace mesaac::measures {

class Tversky : public MeasuresBase {
protected:
  float alpha;
  float beta;

public:
  Tversky(float a);

  std::string name() const override { return "Tversky"; }
  float similarity(const shape_defs::BitVector &v1,
                   const shape_defs::BitVector &v2) const override;
};

} // namespace mesaac::measures