//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape_eigen/axis_aligner.hpp"
#include "mesaac_mol/mol.hpp"

#include <Eigen/Geometry>
#include <Eigen/SVD>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace mesaac {
namespace shape_eigen {
void AxisAligner::align_to_axes(mol::Mol &m) {
  align_to_axes(m.mutable_atoms());
}

void AxisAligner::align_to_axes(mol::AtomVector &atoms) {
  // Strategy:
  //   Get mean-centered heavy atom coordinates
  //   Get mean-centered cloud points
  //   Find the axis-aligning rotation matrix using SVD
  //   Transform the original coordinates: mean center and rotate
  if (atoms.size() > 0) {
    PointList centers;
    PointList cloud;
    Transform transform;

    get_atom_points(atoms, centers, false);
    mean_center_points(centers);
    get_mean_centered_cloud(centers, cloud);
    find_axis_align_transform(cloud, transform);

    PointList all_centers;
    Point mean;
    get_atom_points(atoms, centers, false);
    get_mean_center(centers, mean);
    get_atom_points(atoms, all_centers, true);
    untranslate_points(all_centers, mean);
    transform_points(all_centers, transform);
    update_atom_coords(atoms, all_centers);
  }
}

void AxisAligner::get_atom_points(const mol::AtomVector &atoms,
                                  PointList &centers, bool include_hydrogens) {
  centers.clear();
  for (const auto &atom : atoms) {
    if (include_hydrogens || !atom.is_hydrogen()) {
      const auto &pos(atom.pos());
      centers.push_back({pos.x(), pos.y(), pos.z(), atom.radius()});
    }
  }
}

void AxisAligner::mean_center_points(PointList &points) {
  Point mean;
  get_mean_center(points, mean);
  untranslate_points(points, mean);
}

void AxisAligner::get_mean_center(const PointList &points, Point &mean) {
  mean.clear();
  if (points.size() == 0) {
    mean.push_back(0);
    mean.push_back(0);
    mean.push_back(0);
  } else {
    float xsum = 0, ysum = 0, zsum = 0;
    for (const auto &p : points) {
      xsum += p[0];
      ysum += p[1];
      zsum += p[2];
    }
    mean.push_back(xsum / points.size());
    mean.push_back(ysum / points.size());
    mean.push_back(zsum / points.size());
  }
}

void AxisAligner::untranslate_points(PointList &points, const Point &offset) {
  for (auto &p : points) {
    p[0] -= offset[0];
    p[1] -= offset[1];
    p[2] -= offset[2];
  }
}

static inline bool in_atom(const Point &point, const Point &atom,
                           float eps_sqr) {
  const float radius(atom.at(3));
  const float max_bsqr(radius * radius * eps_sqr);
  double dx(point[0] - atom[0]);
  const double dx_sqr = dx * dx;
  // Only bother to calc distance if inside boundary
  if (dx_sqr <= max_bsqr) {
    double dy(point[1] - atom[1]);
    double dz(point[2] - atom[2]);
    float dsqr = dx_sqr + (dy * dy) + (dz * dz);
    return (dsqr <= max_bsqr);
  }
  return false;
}

void AxisAligner::get_mean_centered_cloud(const PointList &centers,
                                          PointList &cloud) {
  cloud.clear();
  if (m_atom_centers_only) {
    for (const auto &center : centers) {
      cloud.push_back(Point{center[0], center[1], center[2]});
    }
    // Atom centers should already be mean-centered
  } else {
    m_volbox.get_points_within_spheres(centers, cloud, 0);
    mean_center_points(cloud);
  }
}

void AxisAligner::update_atom_coords(mol::AtomVector &atoms,
                                     const PointList &atom_centers) {
  if (atoms.size() != atom_centers.size()) {
    ostringstream msg;
    msg << "Atom vector length " << atoms.size()
        << " must equal atom centers length " << atom_centers.size();
    throw length_error(msg.str());
  }

  mol::AtomVector::iterator atom_iter(atoms.begin());
  PointList::const_iterator center_iter(atom_centers.begin());
  for (; atom_iter != atoms.end(); ++atom_iter, ++center_iter) {
    mol::Atom &atom(*atom_iter);
    const Point &center(*center_iter);
    atom.set_pos({center[0], center[1], center[2]});
  }
}

void AxisAligner::transform_points(PointList &points, Transform &vt) {
  typedef Eigen::Vector3f EPoint;
  for (auto &p : points) {
    EPoint untransformed;
    untransformed << p[0], p[1], p[2];
    const EPoint transformed = vt * untransformed;
    p[0] = transformed[0];
    p[1] = transformed[1];
    p[2] = transformed[2];
  }
}

static bool axis_is_mirrored(Transform &vt) {
  // Transform 3 unit "vectors" such that the 3rd is the cross product
  // of the first 2.  After transformation, confirm it is still the
  // cross product.

  // TODO:  Figure out how to use Vector4f and a 4x4 transform
  // matrix.  Eigen may be able to perform better this way because
  // of auto-vectorization packet sizes.
  typedef Eigen::Vector3f EPoint;
  EPoint a, b, c;
  // Why is this epsilon faster than EPoint(1.0, 0.0, 0.0) etc.?
  a << 1.0, 0.0, 0.0;
  b << 0.0, 1.0, 0.0;
  c << 0.0, 0.0, 1.0;
  a = vt * a;
  b = vt * b;
  c = vt * c;
  EPoint xp(a.cross(b));
  return !(c - xp).isZero();
}

static void unmirror_axes(Transform &vt) {
  for (unsigned int i = 0; i != 3; i++) {
    if (!axis_is_mirrored(vt)) {
      break;
    }
    vt(i) *= -1;
    if (axis_is_mirrored(vt)) {
      // Still mirrored?  Back off and try again w. the next
      // axis.
      vt(i) *= -1;
    }
  }
}

void AxisAligner::find_axis_align_transform(const PointList &cloud,
                                            Transform &transform) {
  if (cloud.size() <= 0) {
    // TODO: Instead of failing, just return the identity transform.
    throw invalid_argument("Can't find alignment for empty cloud");
  }

  const unsigned int num_points(cloud.size());
  Eigen::MatrixXf x(num_points, 3);

  // Fill arrays for PCA code
  for (unsigned int i = 0; i != num_points; i++) {
    const Point &curr_point(cloud[i]);
    for (unsigned int j = 0; j != 3; j++) {
      x(i, j) = curr_point[j];
    }
  }
  transform = x.jacobiSvd(Eigen::DecompositionOptions::ComputeFullV)
                  .matrixV()
                  .transpose();
  unmirror_axes(transform);
}
} // namespace shape_eigen
} // namespace mesaac
