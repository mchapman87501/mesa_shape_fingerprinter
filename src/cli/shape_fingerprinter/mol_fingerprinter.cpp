//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mol_fingerprinter.hpp"

#include <cmath>
#include <cstdlib>

using namespace std;

namespace mesaac::shape_fingerprinter {
namespace {
const static float c_flip_matrix[4][3] = {{1.0, 1.0, 1.0}, // Unflipped
                                          {1.0, -1.0, -1.0},
                                          {-1.0, 1.0, -1.0},
                                          {-1.0, -1.0, 1.0}};
const static unsigned int c_flip_matrix_size =
    sizeof(c_flip_matrix) / sizeof(c_flip_matrix[0]);

} // namespace

MolFingerprinter::MolFingerprinter(shape::PointList &hammsEllipsoidCoords,
                                   shape::PointList &hammsSphereCoords,
                                   float epsilonSqr, unsigned int numFolds)
    : m_axis_aligner(hammsSphereCoords, epsilonSqr, true),
      m_volbox(hammsEllipsoidCoords, epsilonSqr), m_num_folds(numFolds),
      m_i_flip(0) {}

void MolFingerprinter::set_molecule(mol::Mol &mol) {
  m_mol = mol;
  m_i_flip = 0;
  m_heavies.clear();
  m_axis_aligner.align_to_axes(m_mol);
  m_axis_aligner.get_atom_points(m_mol.atoms(), m_heavies, false);
}

bool MolFingerprinter::get_next_fp(shape_defs::BitVector &fp) {
  bool result(false);

  if (m_i_flip != c_flip_matrix_size) {
    compute_curr_flip_fingerprint(m_heavies, fp);
    m_i_flip++;
    result = true;
  }
  return result;
}

namespace {
inline void get_flipped_points(const shape::PointList &points,
                               const float *flip,
                               shape::PointList &flippedPoints) {
  flippedPoints = points;

  shape::PointList::iterator iEnd(flippedPoints.end());
  shape::PointList::iterator i;
  for (i = flippedPoints.begin(); i != iEnd; ++i) {
    shape::Point &p(*i);
    p[0] *= flip[0];
    p[1] *= flip[1];
    p[2] *= flip[2];
  }
}

} // namespace

void MolFingerprinter::compute_curr_flip_fingerprint(
    const shape::PointList &points, shape_defs::BitVector &result) {
  const float *flip = c_flip_matrix[m_i_flip];
  shape::PointList flippedPoints;
  get_flipped_points(points, flip, flippedPoints);

  result.resize(m_volbox.size() / (1 << m_num_folds));
  result.reset();
  m_volbox.set_folded_bits_for_spheres(flippedPoints, result, m_num_folds, 0);
}
} // namespace mesaac::shape_fingerprinter
