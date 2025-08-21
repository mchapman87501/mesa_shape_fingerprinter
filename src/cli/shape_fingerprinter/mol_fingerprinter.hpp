//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

// Singular value decomposition, for PCA -- this defines ap::real_2d_array
#include "svd.h"

#include "mesaac_mol/mol.hpp"
#include "mesaac_shape/axis_aligner.hpp"
#include "mesaac_shape/shared_types.hpp"

namespace mesaac::shape_fingerprinter {
class MolFingerprinter {
public:
  MolFingerprinter(shape::Point3DList &hamms_ellipsoid_coords,
                   shape::Point3DList &hamms_sphere_coords, float epsilon_sqr,
                   unsigned int numFolds);

  /// @brief Set the molecule for which to compute fingerprints.
  /// @param mol the molecule for which to compute subsequent fingerprints
  void set_molecule(mol::Mol &mol);

  /// @brief Get the next fingerprint for the current molecule.  Multiple
  /// fingerprints may be obtained, one for each orientation ("flip") of the
  /// molecule.
  /// @param fp on successful return, the next fingerprint
  /// @return `true` if `fp` was successfully computed, `false`
  /// otherwise.  **Note:** if the return value is `false`, then the state of
  /// `fp` is indeterminate.
  bool get_next_fp(shape_defs::BitVector &fp);

protected:
  shape::AxisAligner m_axis_aligner;
  shape::VolBox m_volbox;
  unsigned int m_num_folds;

  mol::Mol m_mol;
  unsigned int m_i_flip;
  shape::SphereList m_heavies;

  void compute_curr_flip_fingerprint(const shape::SphereList &points,
                                     shape_defs::BitVector &result);
};
} // namespace mesaac::shape_fingerprinter
