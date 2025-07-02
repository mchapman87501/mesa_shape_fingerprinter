#pragma once

#include "mesaac_common/shape_defs.hpp" // For BitVector
#include "mesaac_shape/shared_types.hpp"
#include <memory>
#include <vector>

namespace mesaac::shape {
// Vector of indices, i.e. indices of points which lie within a volume.
using IndexList = std::vector<unsigned int>;
struct IndexedPoint {
  unsigned int index;
  float x, y, z;
};
using IndexedPointList = std::vector<IndexedPoint>;

using ZBucket = std::vector<IndexList>;
using YZBucket = std::vector<ZBucket>;
using XYZBucket = std::vector<YZBucket>;

class VolBox {
public:
  using VolBoxPtr = std::shared_ptr<VolBox>;

  VolBox() {}
  VolBox(const PointList &points, const float sphere_scale);

  // Get the number of points within this VolBox.
  unsigned int size();

  // OBS:  spheres should be a list of 4-membered Points:
  //       x, y, z, radius
  // If from_scratch is true, then bits are cleared and resized to
  // match self's number of points.
  void set_bits_for_spheres(const PointList &spheres,
                            shape_defs::BitVector &bits, bool from_scratch,
                            unsigned int offset) const;

  void set_bits_for_one_sphere(const Point &sphere, shape_defs::BitVector &bits,
                               unsigned int offset) const;

  void get_points_within_spheres(const PointList &spheres,
                                 PointList &contained_points,
                                 unsigned int offset) const;

  // For folded fingerprints:
  void set_folded_bits_for_spheres(const PointList &spheres,
                                   shape_defs::BitVector &bits,
                                   unsigned int num_folds,
                                   unsigned int offset) const;

protected:
  float m_sphere_scale;
  float m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax;
  float m_units_per_side;
  float m_dx, m_dy, m_dz;
  int m_ixmax, m_iymax, m_izmax;

  XYZBucket m_bucket;
  PointList m_bucket_points;

  void add_points(const PointList &points);
  void set_bits_for_one_sphere_unchecked(const Point &sphere,
                                         shape_defs::BitVector &bits,
                                         unsigned int offset) const;
  void set_folded_bits_for_one_sphere_unchecked(const Point &sphere,
                                                shape_defs::BitVector &bits,
                                                unsigned int offset,
                                                unsigned int folded_size) const;
  void get_points_in_cube(float x, float y, float z, float radius,
                          IndexList &result) const;
};
} // namespace mesaac::shape
