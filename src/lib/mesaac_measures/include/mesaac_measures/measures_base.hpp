// Program Measures
// Measures.h
// Measures classes

#pragma once

#include <memory>
#include <string>

#include "mesaac_common/shape_defs.hpp"

namespace mesaac::measures {

// Base class for all measures computations -- provides Tanimoto measure.
class MeasuresBase {
public:
  using Ptr = std::shared_ptr<MeasuresBase>;

  virtual std::string name() const { return "Tanimoto"; }

  float operator()(const shape_defs::BitVector &v1,
                   const shape_defs::BitVector &v2) const {
    return similarity(v1, v2);
  }

  /**
   * @brief Get the similarity measure for two bit vectors.
   * @param v1 first bit vector
   * @param v2 second bit vector
   * @return the similarity measure for bit vectors `v1` and `v2`
   */
  virtual float similarity(const shape_defs::BitVector &v1,
                           const shape_defs::BitVector &v2) const;

  /**
   * @brief Get the distance measure for two bit vectors.
   * @param v1 first bit vector
   * @param v2 second bit vector
   * @return the distance measure for bit vectors `v1` and `v2`
   */
  virtual float distance(const shape_defs::BitVector &v1,
                         const shape_defs::BitVector &v2) const {
    return 1.0 - similarity(v1, v2);
  }
};

} // namespace mesaac::measures