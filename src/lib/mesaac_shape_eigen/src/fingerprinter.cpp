// Shape fingerprint generator, suitable for use with conformers which are
// already consistently aligned.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape_eigen/fingerprinter.hpp"

using namespace std;
using namespace mesaac::mol;

namespace mesaac::shape_eigen {
// Implementation is derived from ShapeFingerprint's mol_fingerprinter.

namespace {

const static float c_flip_matrix[4][3] = {{1.0, 1.0, 1.0}, // Unflipped
                                          {1.0, -1.0, -1.0},
                                          {-1.0, 1.0, -1.0},
                                          {-1.0, -1.0, 1.0}};
const static unsigned int c_flip_matrix_size =
    sizeof(c_flip_matrix) / sizeof(c_flip_matrix[0]);

inline void get_point_list(const AtomVector &atoms, PointList &result) {
  result.clear();
  result.reserve(atoms.size());
  for (const Atom &atom : atoms) {
    const auto &pos(atom.pos());
    result.push_back({pos.x(), pos.y(), pos.z(), atom.radius()});
  }
}

inline void get_flipped_points(const PointList &points, const float *flip,
                               PointList &flipped) {
  flipped = points;
  for (auto &point : flipped) {
    point[0] *= flip[0];
    point[1] *= flip[1];
    point[2] *= flip[2];
  }
}
} // namespace

Fingerprinter::Fingerprinter(const VolBox &volbox) : m_volbox(volbox) {}

void Fingerprinter::compute(const AtomVector &atoms, ShapeFingerprint &result) {
  result.clear();
  result.reserve(c_flip_matrix_size);
  PointList centers;
  get_point_list(atoms, centers);
  for (unsigned int i = 0; i != c_flip_matrix_size; i++) {
    shape_defs::BitVector curr_fp;
    compute_for_flip(centers, i, curr_fp);
    result.push_back(curr_fp);
  }
}

void Fingerprinter::compute_for_flip(const PointList &points,
                                     unsigned int i_flip, Fingerprint &result) {
  result.reset();
  PointList flipped;
  get_flipped_points(points, c_flip_matrix[i_flip], flipped);
  m_volbox.set_bits_for_spheres(flipped, result, true, 0);
}

} // namespace mesaac::shape_eigen
