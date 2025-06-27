//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape_eigen/vol_box.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace mesaac {
namespace shape_eigen {

VolBox::VolBox(const PointList &points, const float sphere_scale) {
  m_units_per_side = 8;
  m_sphere_scale = sphere_scale;

  // TODO:  Factor construction into multiple methods

  // Find the bounding box of all points.
  m_xmin = m_xmax = m_ymin = m_ymax = m_zmin = m_zmax = 0;
  bool first = true;
  for (const auto p : points) {
    if (first) {
      m_xmin = m_xmax = p[0];
      m_ymin = m_ymax = p[1];
      m_zmin = m_zmax = p[2];
      first = false;
    } else {
      m_xmin = min(m_xmin, p[0]);
      m_xmax = max(m_xmax, p[0]);
      m_ymin = min(m_ymin, p[1]);
      m_ymax = max(m_ymax, p[1]);
      m_zmin = min(m_zmin, p[2]);
      m_zmax = max(m_zmax, p[2]);
    }
  }

  m_dx = (m_xmax - m_xmin) / m_units_per_side;
  m_dy = (m_ymax - m_ymin) / m_units_per_side;
  m_dz = (m_zmax - m_zmin) / m_units_per_side;

  m_ixmax = (0 == m_dx) ? 0 : (int)((m_xmax - m_xmin) / m_dx) - 1;
  m_iymax = (0 == m_dy) ? 0 : (int)((m_ymax - m_ymin) / m_dy) - 1;
  m_izmax = (0 == m_dz) ? 0 : (int)((m_zmax - m_zmin) / m_dz) - 1;

  m_ixmax = max(m_ixmax, 0);
  m_iymax = max(m_iymax, 0);
  m_izmax = max(m_izmax, 0);

  m_bucket.clear();

  for (int ix = 0; ix <= m_ixmax; ix++) {
    m_bucket.push_back(YZBucket());
    YZBucket &yz_bucket(m_bucket[ix]);
    for (int iy = 0; iy <= m_iymax; iy++) {
      yz_bucket.push_back(ZBucket());
      ZBucket &z_bucket(yz_bucket[iy]);
      for (int iz = 0; iz <= m_izmax; iz++) {
        z_bucket.push_back(IndexList());
      }
    }
  }

  // Fill the bucket.
  add_points(points);
}

void VolBox::add_points(const PointList &points) {
  m_bucket_points.clear();
  XYZBucket &xyz_bucket(m_bucket);
  for (const auto &p : points) {
    int ix = (int)((p[0] - m_xmin) / m_dx), iy = (int)((p[1] - m_ymin) / m_dy),
        iz = (int)((p[2] - m_zmin) / m_dz);
    ix = max(0, min(ix, m_ixmax));
    iy = max(0, min(iy, m_iymax));
    iz = max(0, min(iz, m_izmax));
    // The actual points are in m_bucket_points.
    // xyz_bucket simply stores indices of these points.
    xyz_bucket.at(ix).at(iy).at(iz).push_back(m_bucket_points.size());
    m_bucket_points.push_back(p);
  }
}

// Get the number of points within this VolBox.
unsigned int VolBox::size() { return m_bucket_points.size(); }

inline static void validate_bits(shape_defs::BitVector &bits,
                                 unsigned int size) {
  if (bits.size() < size) {
    ostringstream outs;
    outs << "set_bits_for_sphere:  not enough bits (" << bits.size()
         << ").  Need at least " << size << " bits.";
    throw std::invalid_argument(outs.str());
  }
}

void VolBox::get_points_within_spheres(const PointList &spheres,
                                       PointList &contained_points,
                                       unsigned int offset) const {
  contained_points.clear();
  shape_defs::BitVector which_points;
  set_bits_for_spheres(spheres, which_points, true, offset);
  contained_points.reserve(which_points.count());
  unsigned int i_max(which_points.size());
  for (unsigned int i = 0; i != i_max; i++) {
    if (which_points.test(i)) {
      contained_points.push_back(m_bucket_points.at(i));
    }
  }
}

void VolBox::set_bits_for_spheres(const PointList &spheres,
                                  shape_defs::BitVector &bits,
                                  bool from_scratch,
                                  unsigned int offset) const {
  if (from_scratch) {
    bits.resize(m_bucket_points.size() + offset);
    bits.reset();
  } else {
    validate_bits(bits, m_bucket_points.size() + offset);
  }

  PointList::const_iterator i;
  for (i = spheres.begin(); i != spheres.end(); ++i) {
    set_bits_for_one_sphere_unchecked(*i, bits, offset);
  }
}

void VolBox::set_bits_for_one_sphere(const Point &sphere,
                                     shape_defs::BitVector &bits,
                                     unsigned int offset) const {
  validate_bits(bits, m_bucket_points.size());
  set_bits_for_one_sphere_unchecked(sphere, bits, offset);
}

void VolBox::set_bits_for_one_sphere_unchecked(const Point &sphere,
                                               shape_defs::BitVector &bits,
                                               unsigned int offset) const {
  // Find all buckets overlapped by the sphere.
  const float x = sphere[0], y = sphere[1], z = sphere[2],
              radius = sphere.at(3) * m_sphere_scale, rsqr = radius * radius;
  IndexList pic; // (indices of) points in cube
  get_points_in_cube(x, y, z, radius, pic);
  IndexList::iterator i;
  IndexList::iterator i_end(pic.end());
  for (i = pic.begin(); i != i_end; i++) {
    unsigned int point_index(*i);

    if (!bits.test(point_index + offset)) {
      const Point &p(m_bucket_points[point_index]);
      float dx = p[0] - x, dy = p[1] - y, dz = p[2] - z,
            ds = (dx * dx + dy * dy + dz * dz);
      if (ds <= rsqr) {
        bits.set(point_index + offset);
      }
    }
  }
}

static inline int bounded(int index, int max_index) {
  return ((index < 0) ? 0 : (index > max_index) ? max_index : index);
}

void VolBox::get_points_in_cube(float x, float y, float z, float radius,
                                IndexList &result) const {
  result.clear();

  int x0 = (int)((x - radius - m_xmin) / m_dx),
      xf = (int)((x + radius - m_xmin) / m_dx),
      y0 = (int)((y - radius - m_ymin) / m_dy),
      yf = (int)((y + radius - m_ymin) / m_dy),
      z0 = (int)((z - radius - m_zmin) / m_dz),
      zf = (int)((z + radius - m_zmin) / m_dz);

  x0 = bounded(x0, m_ixmax);
  xf = bounded(xf, m_ixmax);
  y0 = bounded(y0, m_iymax);
  yf = bounded(yf, m_iymax);
  z0 = bounded(z0, m_izmax);
  zf = bounded(zf, m_izmax);

  for (int x = x0; x <= xf; x++) {
    for (int y = y0; y <= yf; y++) {
      for (int z = z0; z <= zf; z++) {
        const IndexList &pl(m_bucket.at(x).at(y).at(z));
        result.insert(result.end(), pl.begin(), pl.end());
      }
    }
  }
}
} // namespace shape_eigen
} // namespace mesaac
