//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape/axis_aligner.hpp"
#include "mesaac_mol.h"

#include <sstream>
#include <stdexcept>

using namespace std;

namespace mesaac {
namespace shape {
AxisAligner::AxisAligner(const PointList &sphere, float atom_scale,
                         bool atom_centers_only)
    : m_volbox(sphere, atom_scale), m_atom_scale(atom_scale),
      m_atom_centers_only(atom_centers_only) {
  // cerr << "Align to atom centers: " << m_atom_centers_only << endl;
}

// AxisAligner::AxisAligner(const PointList& sphere, float atom_scale):
//     m_volbox(sphere, atom_scale), m_atom_scale(atom_scale),
//     m_atom_centers_only(false)
// {
// }
//
AxisAligner::~AxisAligner() {}

AxisAligner::AxisAligner(const AxisAligner &src) : m_volbox(src.m_volbox) {
  *this = src;
}

AxisAligner &AxisAligner::operator=(const AxisAligner &src) {
  m_volbox = src.m_volbox;
  m_atom_scale = src.m_atom_scale;
  m_atom_centers_only = src.m_atom_centers_only;
  return *this;
}

void AxisAligner::align_to_axes(mol::Mol &m) { align_to_axes(m.atoms()); }

void AxisAligner::align_to_axes(const mol::AtomVector &atoms) {
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
  // TODO:  Separate implementations for include/exclude
  // hydrogens, to eliminate the test on each loop.
  centers.clear();
  mol::AtomVector::const_iterator i;
  for (i = atoms.begin(); i != atoms.end(); ++i) {
    const mol::Atom *a(*i);
    if (a) {
      if (include_hydrogens || !a->is_hydrogen()) {
        Point p;
        p.push_back(a->x());
        p.push_back(a->y());
        p.push_back(a->z());
        p.push_back(a->radius());
        centers.push_back(p);
      }
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
    PointList::const_iterator i;
    for (i = points.begin(); i != points.end(); ++i) {
      const Point &p(*i);
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
  PointList::iterator i;
  for (i = points.begin(); i != points.end(); ++i) {
    Point &p(*i);
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
    PointList::const_iterator i;
    for (i = centers.begin(); i != centers.end(); i++) {
      const Point &c(*i);
      Point p_cloud;
      p_cloud.push_back(c[0]);
      p_cloud.push_back(c[1]);
      p_cloud.push_back(c[2]);
      cloud.push_back(p_cloud);
    }
    // Atom centers should already be mean-centered
  } else {
    m_volbox.get_points_within_spheres(centers, cloud, 0);
    mean_center_points(cloud);
  }
}

void AxisAligner::update_atom_coords(const mol::AtomVector &atoms,
                                     const PointList &atom_centers) {
  if (atoms.size() != atom_centers.size()) {
    ostringstream msg;
    msg << "Atom vector length " << atoms.size()
        << " must equal atom centers length " << atom_centers.size();
    throw length_error(msg.str());
  }

  if (atoms.size() > 0) {
    int i;
    for (i = atoms.size() - 1; i >= 0; i--) {
      mol::Atom *a(atoms[i]);
      if (a) {
        const Point &c(atom_centers[i]);
        a->x(c[0]);
        a->y(c[1]);
        a->z(c[2]);
      }
    }
  }
}

static inline void transform_point(Transform &vt, Point &p) {
  Point untransformed(p);
  for (unsigned int j = 0; j != 3; j++) {
    p[j] = ((vt(j, 0) * untransformed[0]) + (vt(j, 1) * untransformed[1]) +
            (vt(j, 2) * untransformed[2]));
  }
}

void AxisAligner::transform_points(PointList &points, Transform &vt) {
  PointList::iterator i;
  for (i = points.begin(); i != points.end(); ++i) {
    transform_point(vt, *i);
  }
}

static inline void get_cross_prod(const Point &a, const Point &b, Point &xp) {
  xp.clear();
  xp.resize(3);
  xp[0] = (a[1] * b[2] - a[2] * b[1]);
  xp[1] = (-(a[0] * b[2] - a[2] * b[0]));
  xp[2] = (a[0] * b[1] - a[1] * b[0]);
}

// rmatrixsvd may produce a transform matrix which mirrors one of
// the coordinate axes.  Detect this, and fix it.
static inline bool approx(float a, float b) { return (::fabs(a - b) < 1.0e-6); }

static bool axis_is_mirrored(Transform &vt) {
  // Transform 3 unit "vectors" such that the 3rd is the cross product
  // of the first 2.  After transformation, confirm it is still the
  // cross product.
  Point a(3, 0.0), b(3, 0.0), c(3, 0.0);
  a[0] = 1.0;
  b[1] = 1.0;
  c[2] = 1.0;
  transform_point(vt, a);
  transform_point(vt, b);
  transform_point(vt, c);

  Point xp;
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

static inline void unmirror_axis(Transform &vt, unsigned int i_axis) {
  for (unsigned int i = 0; i != 3; i++) {
    vt(i_axis, i) = -vt(i_axis, i);
  }
}

static void unmirror_axes(Transform &vt) {
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

void AxisAligner::find_axis_align_transform(const PointList &cloud,
                                            Transform &transform) {
  if (cloud.size() <= 0) {
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
    const Point &curr_point(cloud[i]);
    for (unsigned int j = 0; j != 3; j++) {
      x(i, j) = curr_point[j];
    }
  }

  if (!rmatrixsvd(x, num_points, 3, 2, 2, 2, w, u, transform)) {
    throw runtime_error("PCA failed.");
  }

  unmirror_axes(transform);
}
} // namespace shape
} // namespace mesaac
