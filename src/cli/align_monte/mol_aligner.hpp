//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

// Singular value decomposition, for PCA -- this defines ap::real_2d_array
#include "svd.h"

#include "mesaac_measures/measures_base.hpp"
#include "mesaac_mol/mol.hpp"
#include "mesaac_shape/axis_aligner.hpp"
#include "mesaac_shape/vol_box.hpp"
#include "shared_types.hpp"

namespace mesaac::align_monte {
class MolAligner {
public:
  MolAligner(PointList &hamms_sphere_coords, float epsilon_sqr,
             shape_defs::BitVector &ref_fp, bool atom_centers_only,
             MeasuresList &measures)
      : m_ref_fingerprint(ref_fp),
        m_axisAligner(hamms_sphere_coords, epsilon_sqr, atom_centers_only),
        m_volBox(hamms_sphere_coords, epsilon_sqr), m_measures(measures) {}

  void process_ref_molecule(mol::Mol &mol, shape_defs::BitVector &ref_fp);
  void process_one_molecule(mol::Mol &mol);

protected:
  const shape_defs::BitVector &m_ref_fingerprint;
  shape::AxisAligner m_axisAligner;
  shape::VolBox m_volBox;
  MeasuresList &m_measures;

  void compute_best_sphere_fingerprint(const PointList &points,
                                       measures::MeasuresBase::Ptr measure,
                                       unsigned int &i_best,
                                       float &best_measure);
  float compute_measure_for_flip(const PointList &points, const float *flip,
                                 measures::MeasuresBase::Ptr measure);
  void flip_mol(mol::Mol &mol, const float *flip);

private:
  explicit MolAligner(const MolAligner &src);
  MolAligner &operator=(const MolAligner &src);
};
} // namespace mesaac::align_monte
