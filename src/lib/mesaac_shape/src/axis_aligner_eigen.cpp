//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape/axis_aligner_eigen.hpp"
#include "mesaac_mol/mol.hpp"

#include <Eigen/Geometry>
#include <Eigen/SVD>

#include <format>
#include <stdexcept>

using namespace std;

namespace mesaac::shape {

void AxisAlignerEigen::align_to_axes(mol::Mol &m) {
  align_to_axes(m.mutable_atoms());
}

void AxisAlignerEigen::align_to_axes(mol::AtomVector &atoms) {
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

void AxisAlignerEigen::get_atom_points(const mol::AtomVector &atoms,
                                       PointList &centers,
                                       bool include_hydrogens) {
  centers.clear();
  for (const auto &atom : atoms) {
    if (include_hydrogens || !atom.is_hydrogen()) {
      const auto &pos(atom.pos());
      centers.push_back({pos.x(), pos.y(), pos.z(), atom.radius()});
    }
  }
}

void AxisAlignerEigen::mean_center_points(PointList &points) {
  Point mean;
  get_mean_center(points, mean);
  untranslate_points(points, mean);
}

void AxisAlignerEigen::get_mean_center(const PointList &points, Point &mean) {
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

void AxisAlignerEigen::untranslate_points(PointList &points,
                                          const Point &offset) {
  for (auto &point : points) {
    point[0] -= offset[0];
    point[1] -= offset[1];
    point[2] -= offset[2];
  }
}

void AxisAlignerEigen::get_mean_centered_cloud(const PointList &centers,
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

void AxisAlignerEigen::update_atom_coords(mol::AtomVector &atoms,
                                          const PointList &atom_centers) {
  if (atoms.size() != atom_centers.size()) {
    throw std::length_error(
        std::format("Atom vector length {} must equal atom centers length {}",
                    atoms.size(), atom_centers.size()));
  }

  mol::AtomVector::iterator atom_iter(atoms.begin());
  PointList::const_iterator center_iter(atom_centers.begin());
  for (; atom_iter != atoms.end(); ++atom_iter, ++center_iter) {
    mol::Atom &atom(*atom_iter);
    const Point &center(*center_iter);
    atom.set_pos({center[0], center[1], center[2]});
  }
}

void AxisAlignerEigen::transform_points(PointList &points, Transform &vt) {
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

// Note: this *should* be a function in an anonymous namespace.
// It's a member function so as to be amenable to unit testing.
void AxisAlignerEigen::unmirror_axes(Transform &transform) {
  const Transform t_orig = transform;

  // Try flipping one axis at a time, until the result has no mirrored axes.
  for (unsigned int i = 0; i != 3; i++) {
    // Is any axis of transform flipped/mirrored?  If not (and if transform
    // doesn't scale any axis to zero) all is good.
    if (transform.determinant() > 0) {
      break;
    }

    Transform axis_flip = Transform::Identity();
    axis_flip(i, i) = -1;
    transform = axis_flip * t_orig;
  }

  if (transform.determinant() < 0) {
    throw std::runtime_error("Could not unmirror axes.");
  }
}

void AxisAlignerEigen::find_axis_align_transform(const PointList &cloud,
                                                 Transform &transform) {
  if (cloud.empty()) {
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
} // namespace mesaac::shape
