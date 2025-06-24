//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape/vol_box.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

using namespace std;

namespace mesaac {
namespace shape {
typedef vector<IndexList> ZBucket;
typedef vector<ZBucket> YZBucket;
struct XYZBucket : vector<YZBucket> {};

VolBox::VolBox(const PointList &points, const float sphere_scale) {
  m_units_per_side = 8;
  m_sphere_scale = sphere_scale;

  // TODO:  Factor construction into multiple methods

  // Find the bounding box of all points.
  m_xmin = m_xmax = m_ymin = m_ymax = m_zmin = m_zmax = 0;
  bool first = true;
  PointList::const_iterator i;
  for (i = points.begin(); i != points.end(); i++) {
    const Point &p(*i);
    if (first) {
      m_xmin = m_xmax = p[0];
      m_ymin = m_ymax = p[1];
      m_zmin = m_zmax = p[2];
      first = false;
    } else {
      m_xmin = (m_xmin < p[0]) ? m_xmin : p[0];
      m_xmax = (m_xmax > p[0]) ? m_xmax : p[0];
      m_ymin = (m_ymin < p[1]) ? m_ymin : p[1];
      m_ymax = (m_ymax > p[1]) ? m_ymax : p[1];
      m_zmin = (m_zmin < p[2]) ? m_zmin : p[2];
      m_zmax = (m_zmax > p[2]) ? m_zmax : p[2];
    }
  }

  m_dx = (m_xmax - m_xmin) / m_units_per_side;
  m_dy = (m_ymax - m_ymin) / m_units_per_side;
  m_dz = (m_zmax - m_zmin) / m_units_per_side;

  m_ixmax = (0 == m_dx) ? 0 : (int)((m_xmax - m_xmin) / m_dx) - 1;
  m_iymax = (0 == m_dy) ? 0 : (int)((m_ymax - m_ymin) / m_dy) - 1;
  m_izmax = (0 == m_dz) ? 0 : (int)((m_zmax - m_zmin) / m_dz) - 1;

  m_ixmax = (m_ixmax < 0) ? 0 : m_ixmax;
  m_iymax = (m_iymax < 0) ? 0 : m_iymax;
  m_izmax = (m_izmax < 0) ? 0 : m_izmax;

  // Build an XYZBucket of all points.
  m_bucket = new XYZBucket();
  assert(m_bucket != 0);

  XYZBucket &xyz_bucket(*m_bucket);
  for (int ix = 0; ix <= m_ixmax; ix++) {
    xyz_bucket.push_back(YZBucket());
    YZBucket &yz_bucket(xyz_bucket[ix]);
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

VolBox::~VolBox() {
  if (m_bucket) {
    delete m_bucket;
    m_bucket = 0;
  }
}

VolBox::VolBox(const VolBox &src) {
  m_bucket = 0;
  *this = src;
}

VolBox &VolBox::operator=(const VolBox &src) {
  m_xmin = src.m_xmin;
  m_xmax = src.m_xmax;
  m_ymin = src.m_ymin;
  m_ymax = src.m_ymax;
  m_zmin = src.m_zmin;
  m_zmax = src.m_zmax;
  m_units_per_side = src.m_units_per_side;
  m_dx = src.m_dx;
  m_dy = src.m_dy;
  m_dz = src.m_dz;
  m_ixmax = src.m_ixmax;
  m_iymax = src.m_iymax;
  m_izmax = src.m_izmax;
  m_sphere_scale = src.m_sphere_scale;
  m_bucket_points = src.m_bucket_points;

  if (m_bucket) {
    delete m_bucket;
  }
  m_bucket = new XYZBucket();
  // This is a bear...
  assert(m_bucket != 0);

  // Lucky thing: STL can do a deep copy correctly.
  *m_bucket = *src.m_bucket;

  return *this;
}

void VolBox::add_points(const PointList &points) {
  m_bucket_points.clear();
  XYZBucket &xyz_bucket(*m_bucket);
  PointList::const_iterator i;
  for (i = points.begin(); i != points.end(); i++) {
    const Point &p(*i);
    int ix = (int)((p[0] - m_xmin) / m_dx), iy = (int)((p[1] - m_ymin) / m_dy),
        iz = (int)((p[2] - m_zmin) / m_dz);
    ix = (ix > m_ixmax) ? m_ixmax : (ix < 0) ? 0 : ix;
    iy = (iy > m_iymax) ? m_iymax : (iy < 0) ? 0 : iy;
    iz = (iz > m_izmax) ? m_izmax : (iz < 0) ? 0 : iz;
    // The actual points are in m_bucket_points.
    // xyz_bucket simply stores indices of these points.
    xyz_bucket.at(ix).at(iy).at(iz).push_back(m_bucket_points.size());
    m_bucket_points.push_back(p);
  }
}

// Get the number of points within this VolBox.
unsigned int VolBox::size() { return m_bucket_points.size(); }

inline static void validate_bits(BitVector &bits, unsigned int size) {
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
  BitVector which_points;
  set_bits_for_spheres(spheres, which_points, true, offset);
  contained_points.reserve(which_points.count());
  unsigned int iMax(which_points.size());
  for (unsigned int i = 0; i != iMax; i++) {
    if (which_points.test(i)) {
      contained_points.push_back(m_bucket_points.at(i));
    }
  }
}

void VolBox::set_bits_for_spheres(const PointList &spheres, BitVector &bits,
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

void VolBox::set_folded_bits_for_spheres(const PointList &spheres,
                                         BitVector &bits,
                                         unsigned int num_folds,
                                         unsigned int offset) const {
  unsigned int fold_factor = 1 << num_folds;
  unsigned int folded_size = m_bucket_points.size() / fold_factor;
  validate_bits(bits, offset + folded_size);

  PointList::const_iterator i;
  for (i = spheres.begin(); i != spheres.end(); ++i) {
    set_folded_bits_for_one_sphere_unchecked(*i, bits, offset, folded_size);
  }
}

void VolBox::set_bits_for_one_sphere(const Point &sphere, BitVector &bits,
                                     unsigned int offset) const {
  validate_bits(bits, m_bucket_points.size());
  set_bits_for_one_sphere_unchecked(sphere, bits, offset);
}

void VolBox::set_bits_for_one_sphere_unchecked(const Point &sphere,
                                               BitVector &bits,
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

void VolBox::set_folded_bits_for_one_sphere_unchecked(
    const Point &sphere, BitVector &bits, unsigned int offset,
    unsigned int folded_size) const {
  // Find all buckets overlapped by the sphere.
  const float x = sphere[0], y = sphere[1], z = sphere[2],
              radius = sphere.at(3) * m_sphere_scale, rsqr = radius * radius;
  IndexList pic; // (indices of) points in cube
  get_points_in_cube(x, y, z, radius, pic);
  IndexList::iterator i;
  IndexList::iterator i_end(pic.end());
  for (i = pic.begin(); i != i_end; i++) {
    unsigned int point_index(*i);
    unsigned int bit_index = (point_index % folded_size) + offset;

    if (!bits.test(bit_index)) {
      const Point &p(m_bucket_points[point_index]);
      float dx = p[0] - x, dy = p[1] - y, dz = p[2] - z,
            ds = (dx * dx + dy * dy + dz * dz);
      if (ds <= rsqr) {
        bits.set(bit_index);
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

  XYZBucket &xyz_bucket(*m_bucket);
  for (int x = x0; x <= xf; x++) {
    for (int y = y0; y <= yf; y++) {
      for (int z = z0; z <= zf; z++) {
        const IndexList &pl(xyz_bucket.at(x).at(y).at(z));
        result.insert(result.end(), pl.begin(), pl.end());
      }
    }
  }
}
} // namespace shape
} // namespace mesaac
