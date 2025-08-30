//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mol_aligner.hpp"

#include <iostream>
#include <sstream>

#include <cmath>
#include <cstdlib>

#include "mesaac_mol/element_info.hpp"

using namespace std;

namespace mesaac::align_monte {
namespace {
// TODO extract to a shared header.
constexpr float c_flip_matrix[4][3] = {{1.0, 1.0, 1.0}, // Unflipped
                                       {1.0, -1.0, -1.0},
                                       {-1.0, 1.0, -1.0},
                                       {-1.0, -1.0, 1.0}};
constexpr unsigned int c_flip_matrix_size =
    sizeof(c_flip_matrix) / sizeof(c_flip_matrix[0]);

void add_tag(mol::Mol &mol, string tag, string value) {
  mol.mutable_tags().add(tag, value);
}

template <typename T> void add_tag(mol::Mol &mol, string tag, T value) {
  ostringstream outs;
  outs << value;
  mol.mutable_tags().add(tag, outs.str());
}

void add_best_measure_tag(mol::Mol &mol, string measure_name, float value) {
  add_tag(mol, "MaxAlign" + measure_name, value);
}

void add_best_flip_tag(mol::Mol &mol, string measure_name, unsigned int value) {
  add_tag(mol, "BestFlip" + measure_name, value);
}

void get_flipped_points(const shape::SphereList &points, const float *flip,
                        shape::SphereList &flipped_points) {
  flipped_points = points;
  for (auto &p : flipped_points) {
    p[0] *= flip[0];
    p[1] *= flip[1];
    p[2] *= flip[2];
  }
}

} // namespace

// Process a reference molecule, returning its fingerprint in ref_fp.
void MolAligner::process_ref_molecule(mol::Mol &mol,
                                      shape_defs::BitVector &ref_fp) {
  // coords holds aligned coordinates for all heavy atoms.
  shape::SphereList heavies;

  m_axisAligner.align_to_axes(mol);
  m_axisAligner.get_atom_points(mol.atoms(), heavies, false);

  m_volBox.set_bits_for_spheres(heavies, ref_fp, true, 0);

  for (const auto &curr_measure : m_measures) {
    if (curr_measure != nullptr) {
      const string name(curr_measure->name());
      add_best_measure_tag(mol, name, 1.0);
      add_best_flip_tag(mol, name, 0);
    }
  }
}

void MolAligner::process_one_molecule(mol::Mol &mol) {
  m_axisAligner.align_to_axes(mol);

  shape::SphereList heavies;
  m_axisAligner.get_atom_points(mol.atoms(), heavies, false);

  // The last measure wins, flip-wise?
  unsigned int best_flip = 0;
  for (const auto &measure : m_measures) {
    if (measure != nullptr) {
      string name = measure->name();
      float best_measure = 0.0;

      compute_best_sphere_fingerprint(heavies, measure, best_flip,
                                      best_measure);
      add_best_measure_tag(mol, name, best_measure);
      add_best_flip_tag(mol, name, best_flip);
    }
  }
  flip_mol(mol, c_flip_matrix[best_flip]);
}

void MolAligner::compute_best_sphere_fingerprint(
    const shape::SphereList &points, measures::MeasuresBase::Ptr measure,
    unsigned int &i_best, float &best_measure) {
  i_best = 0;
  best_measure = 0;
  for (unsigned int i_flip = 0; i_flip != c_flip_matrix_size; i_flip++) {
    const float *flip = c_flip_matrix[i_flip];
    const float curr_measure = compute_measure_for_flip(points, flip, measure);
    if (curr_measure > best_measure) {
      i_best = i_flip;
      best_measure = curr_measure;
    }
  }
}

float MolAligner::compute_measure_for_flip(
    const shape::SphereList &points, const float *flip,
    measures::MeasuresBase::Ptr measure) {
  shape::SphereList flipped_points;
  get_flipped_points(points, flip, flipped_points);

  shape_defs::BitVector curr_fingerprint;
  m_volBox.set_bits_for_spheres(flipped_points, curr_fingerprint, true, 0);
  return measure->similarity(curr_fingerprint, m_ref_fingerprint);
}

void MolAligner::flip_mol(mol::Mol &mol, const float *flip) {
  for (auto &atom : mol.mutable_atoms()) {
    const auto pos = atom.pos();
    atom.set_pos({pos.x() * flip[0], pos.y() * flip[1], pos.z() * flip[2]});
  }
}

} // namespace mesaac::align_monte
