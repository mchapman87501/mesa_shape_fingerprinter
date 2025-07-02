// Shape fingerprint generator, suitable for use with conformers which are
// already consistently aligned.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_common/shape_defs.hpp"
#include "mesaac_mol/atom.hpp"
#include "mesaac_shape_eigen/vol_box.hpp"

namespace mesaac::shape_eigen {

class Fingerprinter {
public:
  Fingerprinter(const VolBox &volbox);

  void compute(const mol::AtomVector &atoms, ShapeFingerprint &result);

protected:
  const VolBox &m_volbox;

  void compute_for_flip(const PointList &points, unsigned int i_flip,
                        Fingerprint &result);

private:
  Fingerprinter(const Fingerprinter &src);
  Fingerprinter &operator=(const Fingerprinter &src);
};

} // namespace mesaac::shape_eigen
