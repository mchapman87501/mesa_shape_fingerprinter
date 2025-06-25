// Shape fingerprint generator, suitable for use with conformers which are
// already consistently aligned.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape/fingerprinter.hpp"

using namespace std;
using namespace mesaac::mol;

namespace mesaac {
namespace shape {
// Implementation is derived from ShapeFingerprint's mol_fingerprinter.

const static float C_FlipMatrix[4][3] = {{1.0, 1.0, 1.0}, // Unflipped
                                         {1.0, -1.0, -1.0},
                                         {-1.0, 1.0, -1.0},
                                         {-1.0, -1.0, 1.0}};
const static unsigned int C_FlipMatrixSize =
    sizeof(C_FlipMatrix) / sizeof(C_FlipMatrix[0]);

Fingerprinter::Fingerprinter(const VolBox &volbox) : m_volbox(volbox) {}

Fingerprinter::~Fingerprinter() {}

static inline void get_point_list(const AtomVector &atoms, PointList &result) {
  result.clear();
  result.reserve(atoms.size());
  for (const Atom &atom : atoms) {
    result.push_back({atom.x(), atom.y(), atom.z(), atom.radius()});
  }
}

void Fingerprinter::compute(const AtomVector &atoms, ShapeFingerprint &result) {
  result.clear();
  result.reserve(C_FlipMatrixSize);
  PointList centers;
  get_point_list(atoms, centers);
  for (unsigned int i = 0; i != C_FlipMatrixSize; i++) {
    BitVector curr_fp;
    compute_for_flip(centers, i, curr_fp);
    result.push_back(curr_fp);
  }
}

static inline void get_flipped_points(const PointList &points,
                                      const float *flip, PointList &flipped) {
  flipped = points;

  PointList::iterator iEnd(flipped.end());
  PointList::iterator i;
  for (i = flipped.begin(); i != iEnd; ++i) {
    Point &p(*i);
    p[0] *= flip[0];
    p[1] *= flip[1];
    p[2] *= flip[2];
  }
}

void Fingerprinter::compute_for_flip(const PointList &points,
                                     unsigned int i_flip, Fingerprint &result) {
  result.reset();
  PointList flipped;
  get_flipped_points(points, C_FlipMatrix[i_flip], flipped);
  m_volbox.set_bits_for_spheres(flipped, result, true, 0);
}

} // namespace shape
} // namespace mesaac
