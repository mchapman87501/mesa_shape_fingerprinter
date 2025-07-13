//
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <memory>

#include "mesaac_measures/shape_measures_factory.hpp"

#include "mesaac_measures/bub.hpp"
#include "mesaac_measures/cosine.hpp"
#include "mesaac_measures/euclidean.hpp"
#include "mesaac_measures/hamann.hpp"
#include "mesaac_measures/measures_base.hpp"
#include "mesaac_measures/tanimoto.hpp"
#include "mesaac_measures/tversky.hpp"

using namespace std;
using namespace mesaac::shape;

namespace mesaac::measures::shape {

namespace {
using MeasuresPtr = MeasuresBase::Ptr;

class Measurer : public IIndexedShapeFPMeasure {

public:
  Measurer(const shape_defs::ArrayBitVectors &fingerprints, MeasuresPtr measure)
      : m_fps(fingerprints), m_measure(measure) {}

  float value(unsigned int i, unsigned int j) const override {
    float result = 1.0;
    if (i != j) {
      result = (*m_measure)(m_fps[i], m_fps[j]);
    }
    return result;
  }

protected:
  const shape_defs::ArrayBitVectors &m_fps;
  const MeasuresPtr m_measure;
};

class DistMeasurer : public IIndexedShapeFPMeasure {
public:
  DistMeasurer(const shape_defs::ArrayBitVectors &fingerprints,
               MeasuresPtr measure)
      : m_fps(fingerprints), m_measure(measure) {}

  float value(unsigned int i, unsigned int j) const {
    float result = 0.0;
    if (i != j) {
      result = 1.0 - (*m_measure)(m_fps[i], m_fps[j]);
    }
    return result;
  }

protected:
  const shape_defs::ArrayBitVectors &m_fps;
  MeasuresPtr m_measure;
};

// This probably belongs in Globals...
const unsigned int ShapeMeasurerBlockSize = 4;

class ShapeMeasurer : public IIndexedShapeFPMeasure {
public:
  ShapeMeasurer(const shape_defs::ShapeFPBlocks &fingerprints,
                MeasuresPtr measure)
      : m_fps(fingerprints), m_measure(measure) {}

  float value(unsigned int i, unsigned int j) const override {
    float result = 1.0;
    if (i != j) {
      MeasuresBase &m(*m_measure);
      const shape_defs::ArrayBitVectors &i_fps(m_fps[i]);
      const shape_defs::ArrayBitVectors &j_fps(m_fps[j]);
      // Check the first fingerprint from group i against all
      // members of group j, looking for the highest similarity.
      result = m(i_fps[0], j_fps[0]);
      for (unsigned int k = 1; k != ShapeMeasurerBlockSize; k++) {
        float pair_result = m(i_fps[0], j_fps[k]);
        result = (result > pair_result) ? result : pair_result;
      }
    }
    return result;
  }

protected:
  const shape_defs::ShapeFPBlocks &m_fps;
  MeasuresPtr m_measure;
};

class ShapeDistMeasurer : public IIndexedShapeFPMeasure {
public:
  ShapeDistMeasurer(const shape_defs::ShapeFPBlocks &fingerprints,
                    MeasuresPtr measure)
      : m_fps(fingerprints), m_measure(measure) {}

  float value(unsigned int i, unsigned int j) const override {
    float result = 0.0;
    if (i != j) {
      MeasuresBase &m(*m_measure);
      const shape_defs::ArrayBitVectors &i_fps(m_fps[i]);
      const shape_defs::ArrayBitVectors &j_fps(m_fps[j]);
      // Check the first fingerprint from group i against all
      // members of group j, looking for the highest similarity.
      float best = m(i_fps[0], j_fps[0]);
      for (unsigned int k = 1; k != ShapeMeasurerBlockSize; k++) {
        float curr_pair = m(i_fps[0], j_fps[k]);
        best = (best > curr_pair) ? best : curr_pair;
      }
      result = 1.0 - best;
    }
    return result;
  }

protected:
  const shape_defs::ShapeFPBlocks &m_fps;
  MeasuresPtr m_measure;
};

// Pairwise measurers are for clients which do not have full shape
// fingerprints arrays.
class ShapePairMeasurer : public IShapeFPMeasure {
public:
  ShapePairMeasurer(MeasuresPtr measure) : m_measure(measure) {}

  float value(const ShapeFingerprint &sfp1,
              const ShapeFingerprint &sfp2) const override {
    MeasuresBase &m(*m_measure);
    // Check the first fingerprint from sfp1 against all
    // members of sfp2, looking for the highest similarity.
    float result = m(sfp1[0], sfp2[0]);
    for (unsigned int k = 1; k != ShapeMeasurerBlockSize; k++) {
      float pair_result = m(sfp1[0], sfp2[k]);
      result = max(result, pair_result);
    }
    return result;
  }

protected:
  MeasuresPtr m_measure;
};

class ShapePairDistMeasurer : public IShapeFPMeasure {
public:
  ShapePairDistMeasurer(MeasuresPtr measure) : m_measure(measure) {}

  float value(const ShapeFingerprint &sfp1,
              const ShapeFingerprint &sfp2) const override {
    MeasuresBase &m(*m_measure);
    float best = m(sfp1[0], sfp2[0]);
    for (unsigned int k = 1; k != ShapeMeasurerBlockSize; k++) {
      float curr_pair = m(sfp1[0], sfp2[k]);
      // Find the highest similarity to sfp1[0] -- the least distance.
      best = max(best, curr_pair);
    }
    float result = 1.0 - best;
    return result;
  }

protected:
  MeasuresPtr m_measure;
};
} // namespace

IIndexedShapeFPMeasure::Ptr
get_fp_measurer(MeasuresBase::Ptr measure, bool compute_sim,
                const mesaac::shape::FingerprintVector &fingerprints) {

  // Caller should expect a zero result to mean we couldn't find
  // a measure corresponding to dist_type.
  IIndexedShapeFPMeasure::Ptr result = nullptr;
  if (measure) {
    if (compute_sim) {
      result = std::make_shared<Measurer>(fingerprints, measure);
    } else {
      result = std::make_shared<DistMeasurer>(fingerprints, measure);
    }
  }
  return result;
}

IIndexedShapeFPMeasure::Ptr
get_shape_measurer(MeasuresBase::Ptr measure, bool compute_sim,
                   const mesaac::shape::ShapeFingerprintVector &fingerprints) {

  // Caller should expect a zero result to mean we couldn't find
  // a measure corresponding to dist_type.
  IIndexedShapeFPMeasure::Ptr result = nullptr;
  if (measure) {
    if (compute_sim) {
      result = std::make_shared<ShapeMeasurer>(fingerprints, measure);
    } else {
      result = std::make_shared<ShapeDistMeasurer>(fingerprints, measure);
    }
  }
  return result;
}

IShapeFPMeasure::Ptr get_shape_pair_measurer(MeasuresBase::Ptr measure,
                                             bool compute_sim) {

  // Caller should expect a zero result to mean we couldn't find
  // a measure corresponding to dist_type.
  IShapeFPMeasure::Ptr result = nullptr;
  if (measure) {
    if (compute_sim) {
      result = std::make_shared<ShapePairMeasurer>(measure);
    } else {
      result = std::make_shared<ShapePairDistMeasurer>(measure);
    }
  }
  return result;
}
} // namespace mesaac::measures::shape