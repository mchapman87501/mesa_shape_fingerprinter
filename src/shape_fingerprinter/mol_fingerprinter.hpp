//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

// Singular value decomposition, for PCA -- this defines ap::real_2d_array
#include "svd.h"

#include "mesaac_mol.hpp"
#include "mesaac_shape.hpp"
#include "shared_types.hpp"

namespace mesaac::shape_fingerprinter {
class MolFingerprinter {
public:
  MolFingerprinter(PointList &hammsEllipsoidCoords,
                   PointList &hammsSphereCoords, float epsilonSqr,
                   unsigned int numFolds);
  virtual ~MolFingerprinter();

  explicit MolFingerprinter(const MolFingerprinter &src);

  void setMolecule(mol::Mol &mol);
  bool getNextFP(shape_defs::BitVector &fp);

protected:
  shape::AxisAligner m_axisAligner;
  shape::VolBox m_volBox;
  unsigned int m_numFolds;

  mol::Mol m_mol;
  unsigned int m_iFlip;
  PointList m_heavies;

  void computeCurrFlipFingerprint(const PointList &points,
                                  shape_defs::BitVector &result);

private:
  MolFingerprinter &operator=(const MolFingerprinter &src);
};
} // namespace mesaac::shape_fingerprinter
