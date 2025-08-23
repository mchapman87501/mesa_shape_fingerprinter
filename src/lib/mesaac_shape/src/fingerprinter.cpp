// Shape fingerprint generator, suitable for use with conformers which are
// already consistently aligned.
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape/fingerprinter.hpp"

using namespace std;
using namespace mesaac::mol;

namespace mesaac::shape {

namespace {

// Implementation is derived from ShapeFingerprint's mol_fingerprinter.

constexpr float c_flip_matrix[4][3] = {{1.0, 1.0, 1.0}, // Unflipped
                                       {1.0, -1.0, -1.0},
                                       {-1.0, 1.0, -1.0},
                                       {-1.0, -1.0, 1.0}};
constexpr unsigned int c_flip_matrix_size =
    sizeof(c_flip_matrix) / sizeof(c_flip_matrix[0]);

inline void get_point_list(const AtomVector &atoms, SphereList &result) {
  result.clear();
  result.reserve(atoms.size());
  for (const Atom &atom : atoms) {
    const auto &pos(atom.pos());
    result.push_back({pos.x(), pos.y(), pos.z(), atom.radius()});
  }
}

inline void get_flipped_points(const SphereList &centers, const float *flip,
                               SphereList &flipped) {
  flipped = centers;

  for (auto &point : flipped) {
    point[0] *= flip[0];
    point[1] *= flip[1];
    point[2] *= flip[2];
  }
}

void compute_for_flip(const SphereList &centers, unsigned int i_flip,
                      const VolBox &volbox, Fingerprint &result) {
  result.reset();
  SphereList flipped;
  get_flipped_points(centers, c_flip_matrix[i_flip], flipped);
  volbox.set_bits_for_spheres(flipped, result, true, 0);
}

} // namespace

Fingerprinter::Fingerprinter(const VolBox &volbox) : m_volbox(volbox) {}

void Fingerprinter::compute(const AtomVector &atoms, ShapeFingerprint &result) {
  result.clear();
  result.reserve(c_flip_matrix_size);
  SphereList centers;
  get_point_list(atoms, centers);
  for (unsigned int i = 0; i != c_flip_matrix_size; i++) {
    shape_defs::BitVector curr_fp;
    compute_for_flip(centers, i, m_volbox, curr_fp);
    result.push_back(curr_fp);
  }
}

} // namespace mesaac::shape
