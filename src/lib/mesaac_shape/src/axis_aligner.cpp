//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape/axis_aligner.hpp"
#include "mesaac_mol/mol.hpp"

#include <format>
#include <stdexcept>

using namespace std;

namespace mesaac::shape {
namespace {
template <typename PointType>
void transform_point(Transform &vt, PointType &p) {
  PointType untransformed(p);
  for (unsigned int j = 0; j != 3; j++) {
    p[j] = ((vt(j, 0) * untransformed[0]) + (vt(j, 1) * untransformed[1]) +
            (vt(j, 2) * untransformed[2]));
  }
}

inline void get_cross_prod(const Point3D &a, const Point3D &b, Point3D &xp) {
  xp = {0, 0, 0};
  xp[0] = (a[1] * b[2] - a[2] * b[1]);
  xp[1] = (-(a[0] * b[2] - a[2] * b[0]));
  xp[2] = (a[0] * b[1] - a[1] * b[0]);
}

// rmatrixsvd may produce a transform matrix which mirrors one of
// the coordinate axes.  Detect this, and fix it.
inline bool approx(float a, float b) { return (::fabs(a - b) < 1.0e-6); }

bool axis_is_mirrored(Transform &vt) {
  // Transform 3 unit "vectors" such that the 3rd is the cross product
  // of the first 2.  After transformation, confirm it is still the
  // cross product.
  Point3D a{0, 0, 0}, b{0, 0, 0}, c{0, 0, 0};
  a[0] = 1.0;
  b[1] = 1.0;
  c[2] = 1.0;
  transform_point(vt, a);
  transform_point(vt, b);
  transform_point(vt, c);

  Point3D xp;
  get_cross_prod(a, b, xp);

  bool result = false;
  for (unsigned int i = 0; i != 3; i++) {
    if (!approx(c[i], xp[i])) {
      result = true;
      break;
    }
  }
  return result;
}

inline void unmirror_axis(Transform &vt, unsigned int i_axis) {
  for (unsigned int i = 0; i != 3; i++) {
    vt(i_axis, i) = -vt(i_axis, i);
  }
}

void unmirror_axes(Transform &vt) {
  for (unsigned int i = 0; i != 3; i++) {
    if (!axis_is_mirrored(vt)) {
      break;
    }
    unmirror_axis(vt, i);
    if (axis_is_mirrored(vt)) {
      // Still mirrored?  Back off and try again w. the next
      // axis.
      unmirror_axis(vt, i);
    }
  }
}

template <typename PointListType, typename PointType>
void get_mean_center_impl(const PointListType &points, PointType &mean) {
  mean = {0, 0, 0};
  if (!points.empty()) {
    float xsum = 0, ysum = 0, zsum = 0;
    for (const auto &point : points) {
      xsum += point[0];
      ysum += point[1];
      zsum += point[2];
    }
    const auto npts = points.size();
    mean = {xsum / npts, ysum / npts, zsum / npts};
  }
}

template <typename PointListType, typename PointType>
void untranslate_points_impl(PointListType &points, const PointType &offset) {
  for (auto &point : points) {
    point[0] -= offset[0];
    point[1] -= offset[1];
    point[2] -= offset[2];
  }
}

template <typename PointType>
void mean_center_points_impl(std::vector<PointType> &points) {
  PointType mean;
  get_mean_center_impl(points, mean);
  untranslate_points_impl(points, mean);
}
} // namespace

AxisAligner::AxisAligner(const Point3DList &points, float atom_scale,
                         bool atom_centers_only)
    : m_volbox(points, atom_scale), m_atom_scale(atom_scale),
      m_atom_centers_only(atom_centers_only) {}

void AxisAligner::align_to_axes(mol::Mol &m) {
  align_to_axes(m.mutable_atoms());
}

void AxisAligner::align_to_axes(mol::AtomVector &atoms) {
  // Strategy:
  //   Get mean-centered heavy atom coordinates
  //   Get mean-centered cloud points
  //   Find the axis-aligning rotation matrix using SVD
  //   Transform the original coordinates: mean center and rotate
  if (!atoms.empty()) {
    SphereList centers;
    Point3DList cloud;
    Transform transform;

    get_atom_points(atoms, centers, false);
    mean_center_points(centers);
    get_mean_centered_cloud(centers, cloud);
    find_axis_align_transform(cloud, transform);

    SphereList all_centers;
    Point3D mean;
    get_atom_points(atoms, centers, false);
    get_mean_center(centers, mean);
    get_atom_points(atoms, all_centers, true);
    untranslate_points(all_centers, mean);
    transform_points(all_centers, transform);
    update_atom_coords(atoms, all_centers);
  }
}

void AxisAligner::get_atom_points(const mol::AtomVector &atoms,
                                  SphereList &centers, bool include_hydrogens) {
  // TODO:  Separate implementations for include/exclude
  // hydrogens, to eliminate the test on each loop.
  centers.clear();
  for (const auto &atom : atoms) {
    if (include_hydrogens || !atom.is_hydrogen()) {
      const auto &pos(atom.pos());
      centers.push_back({pos.x(), pos.y(), pos.z(), atom.radius()});
    }
  }
}

void AxisAligner::mean_center_points(SphereList &points) {
  mean_center_points_impl(points);
}

void AxisAligner::get_mean_center(const SphereList &points, Point3D &mean) {
  get_mean_center_impl(points, mean);
}

void AxisAligner::untranslate_points(SphereList &points,
                                     const Point3D &offset) {
  untranslate_points_impl(points, offset);
}

void AxisAligner::get_mean_centered_cloud(const SphereList &centers,
                                          Point3DList &cloud) {
  cloud.clear();
  if (m_atom_centers_only) {
    for (const auto &center : centers) {
      cloud.push_back({center[0], center[1], center[2]});
    }
    // Atom centers should already be mean-centered
  } else {
    m_volbox.get_points_within_spheres(centers, cloud, 0);
    mean_center_points_impl(cloud);
  }
}

void AxisAligner::update_atom_coords(mol::AtomVector &atoms,
                                     const SphereList &atom_centers) {
  if (atoms.size() != atom_centers.size()) {
    throw std::length_error(
        std::format("Atom vector length {} must equal atom centers length {}",
                    atoms.size(), atom_centers.size()));
  }

  // Consider C++23 std::ranges::views:zip.
  mol::AtomVector::iterator atom_iter(atoms.begin());
  SphereList::const_iterator center_iter(atom_centers.begin());
  for (; atom_iter != atoms.end(); ++atom_iter, ++center_iter) {
    mol::Atom &atom(*atom_iter);
    const Sphere &center(*center_iter);
    atom.set_pos({center[0], center[1], center[2]});
  }
}

void AxisAligner::transform_points(SphereList &points, Transform &vt) {
  for (auto &point : points) {
    transform_point(vt, point);
  }
}

void AxisAligner::find_axis_align_transform(const Point3DList &cloud,
                                            Transform &transform) {
  if (cloud.empty()) {
    // TODO: Instead of failing, just return the identity transform.
    throw invalid_argument("Can't find alignment for empty cloud");
  }

  Transform x;         // data matrix (input coordinates)
  ap::real_1d_array w; // eigenvalues in sorted descending order
  Transform u;         // eigenvalues not used in this application
  const unsigned int num_points(cloud.size());
  x.setbounds(0, num_points - 1, 0, 2);
  // Fill arrays for PCA code
  for (unsigned int i = 0; i != num_points; i++) {
    const Point3D &curr_point(cloud[i]);
    for (unsigned int j = 0; j != 3; j++) {
      x(i, j) = curr_point[j];
    }
  }

  if (!rmatrixsvd(x, num_points, 3, 2, 2, 2, w, u, transform)) {
    throw runtime_error("PCA failed.");
  }

  unmirror_axes(transform);
}
} // namespace mesaac::shape
