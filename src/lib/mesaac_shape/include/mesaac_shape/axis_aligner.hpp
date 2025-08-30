//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol/mol.hpp"
#include "mesaac_shape/shared_types.hpp"
#include "mesaac_shape/vol_box.hpp"
// Singular value decomposition, for PCA -- this, from dependencies/svd/libs,
// defines ap::real_2d_array
#include "ap.h"

/**
 * @brief Namespace for shape computations using alglib svd.
 */
namespace mesaac::shape {
using Transform = ap::real_2d_array;

class AxisAligner {
public:
  AxisAligner(const Point3DList &points, float atom_scale,
              bool atom_centers_only);

  void align_to_axes(mesaac::mol::Mol &m);
  void align_to_axes(mesaac::mol::AtomVector &atoms);
  void get_atom_points(const mesaac::mol::AtomVector &atoms,
                       SphereList &centers, bool include_hydrogens);

protected:
  VolBox m_volbox;
  float m_atom_scale;
  bool m_atom_centers_only;

  // These really should not be exposed as member functions.
  // They are so exposed to ease unit testing.
  void mean_center_points(SphereList &centers);
  void get_mean_centered_cloud(const SphereList &centers, Point3DList &cloud);
  void find_axis_align_transform(const Point3DList &cloud,
                                 Transform &transform);

  void get_mean_center(const SphereList &centers, Point3D &mean);
  void untranslate_points(SphereList &all_centers, const Point3D &offset);
  void transform_points(SphereList &all_centers, Transform &transform);
  void update_atom_coords(mesaac::mol::AtomVector &atoms,
                          const SphereList &all_centers);
};
} // namespace mesaac::shape
