//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol.hpp"
#include "mesaac_shape/shared_types.hpp"
#include "mesaac_shape/vol_box.hpp"
// Singular value decomposition, for PCA -- this defines ap::real_2d_array
#include "svd.h"

namespace mesaac {
namespace shape {
typedef ap::real_2d_array Transform;

class AxisAligner {
public:
  // AxisAligner(const PointList& sphere, float atom_scale);
  AxisAligner(const PointList &sphere, float atom_scale,
              bool atom_centers_only);
  virtual ~AxisAligner();

  AxisAligner(const AxisAligner &src);
  AxisAligner &operator=(const AxisAligner &src);

  void align_to_axes(mesaac::mol::Mol &m);
  void align_to_axes(const mesaac::mol::AtomVector &atoms);
  void get_atom_points(const mesaac::mol::AtomVector &atoms, PointList &centers,
                       bool include_hydrogens);

protected:
  VolBox m_volbox;
  float m_atom_scale;
  bool m_atom_centers_only;

  // These really should not be exposed as member functions.
  // They are so exposed to ease unit testing.
  void mean_center_points(PointList &centers);
  void get_mean_centered_cloud(const PointList &centers, PointList &cloud);
  void find_axis_align_transform(const PointList &cloud, Transform &transform);

  void get_mean_center(const PointList &centers, Point &mean);
  void untranslate_points(PointList &all_centers, const Point &offset);
  void transform_points(PointList &all_centers, Transform &transform);
  void update_atom_coords(const mesaac::mol::AtomVector &atoms,
                          const PointList &all_centers);
};
} // namespace shape
} // namespace mesaac
