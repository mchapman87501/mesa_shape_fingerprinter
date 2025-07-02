//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

// Singular value decomposition, for PCA -- this defines ap::real_2d_array
#include "svd.h"

#include "mesaac_mol.hpp"
#include "mesaac_shape.hpp"
#include "shared_types.hpp"

class IFeatureFinder;

namespace mesaac {
class MolFingerprinter {
public:
  MolFingerprinter(PointList &hammsEllipsoidCoords,
                   PointList &hammsSphereCoords, float epsilonSqr,
                   unsigned int numFolds);
  virtual ~MolFingerprinter();

  explicit MolFingerprinter(const MolFingerprinter &src);

  void setMolecule(mol::Mol &mol);
  bool getNextFP(BitVector &fp);

protected:
  shape::AxisAligner m_axisAligner;
  shape::VolBox m_volBox;
  unsigned int m_numFolds;

  mol::Mol m_mol;
  IFeatureFinder *m_ff;

  unsigned int m_iFlip;
  PointList m_heavies;
  PointList m_allAtoms;

  void updateFF();
  void computeCurrFlipFingerprint(BitVector &result);

private:
  MolFingerprinter &operator=(const MolFingerprinter &src);
};
} // namespace mesaac
