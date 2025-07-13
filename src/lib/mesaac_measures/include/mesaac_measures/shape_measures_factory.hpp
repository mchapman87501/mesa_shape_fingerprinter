//
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <memory>
#include <string>

#include "mesaac_measures/measures_base.hpp"
#include "mesaac_shape/shared_types.hpp"

namespace mesaac::measures::shape {

/**
 * @brief This is the interface for objects that compute pairs
 * of shape fingerprints.
 */
class IShapeFPMeasure {
public:
  using Ptr = std::shared_ptr<IShapeFPMeasure>;

  /**
   * @brief Get the similarity/distance measure for two shape fingerprints.
   * @param sfp1 a bit vector collection
   * @param sfp2 another bit vector collection
   * @return the similarity/distance measure for the two collections
   * @note An implementation detail: `value` measures only the first
   * element of `sfp1` against the elements of `sfp2` and it returns the
   * "best" (most similar or least distant) measured value.
   */
  virtual float value(const mesaac::shape::ShapeFingerprint &sfp1,
                      const mesaac::shape::ShapeFingerprint &sfp2) const = 0;
};

/**
 * @brief This is the interface for objects that compute measures for
 * two fingerprints, or shape fingerprints, belonging to an indexable
 * collection.
 */
class IIndexedShapeFPMeasure {
public:
  using Ptr = std::shared_ptr<IIndexedShapeFPMeasure>;

  /**
   * @brief Given the indices of two fingerprints,
   * get their siimilarity.
   * @param i index of a shape fingerprint
   * @param j index of another shape fingerprint
   * @return the similarity/distance measure of the two shape fingerprints
   */
  virtual float value(unsigned int i, unsigned int j) const = 0;
};

/**
 * @brief Get an indexed fingerprint measure.
 * @param compute_sim whether to compute similarity or distance values
 * @param dist_type the type of similarity/distance value to compute, e.g., "E"
 * for Euclidean
 * @param fingerprints the collection of fingerprints for which to compute
 * measures
 * @param tversky_alpha the Tversky measure alpha value to use (applicable only
 * for Tversky)
 * @return
 */
IIndexedShapeFPMeasure::Ptr
get_fp_measurer(MeasuresBase::Ptr measure, bool compute_sim,
                const mesaac::shape::FingerprintVector &fingerprints);

IIndexedShapeFPMeasure::Ptr
get_shape_measurer(MeasuresBase::Ptr measure, bool compute_sim,
                   const mesaac::shape::ShapeFingerprintVector &fingerprints);

IShapeFPMeasure::Ptr get_shape_pair_measurer(MeasuresBase::Ptr measure,
                                             bool compute_sim);

} // namespace mesaac::measures::shape