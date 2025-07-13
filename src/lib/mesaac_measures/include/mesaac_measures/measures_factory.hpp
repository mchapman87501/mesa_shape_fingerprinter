#pragma once

#include <string>

#include "mesaac_measures/measures_base.hpp"

namespace mesaac::measures {
enum class MeasureType : unsigned int {
  bub,
  cosine,
  euclidean,
  hamann,
  tanimoto,
  tversky
};

// XXX FIX THIS should be named "get_measure".
MeasuresBase::Ptr get_measures(const MeasureType measure_type,
                               const float tversky_alpha);
} // namespace mesaac::measures
