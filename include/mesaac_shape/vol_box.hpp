#pragma once

#include "shape_defs.hpp" // For BitVector
#include "shared_types.hpp"
#include <memory>
#include <vector>

namespace mesaac {
namespace shape {
// Vector of indices, i.e. indices of points which lie within a volume.
typedef std::vector<unsigned int> IndexList;
struct IndexedPoint {
  unsigned int index;
  float x, y, z;
};
typedef std::vector<IndexedPoint> IndexedPointList;

class XYZBucket;

// When sharing VolBox instances it might be good to use shared
// (ref-counted) pointers:
class VolBox;
typedef std::shared_ptr<VolBox> VolBoxPtr;

class VolBox {
public:
  VolBox(const PointList &points, const float sphere_scale);
  virtual ~VolBox();
  VolBox(const VolBox &src);
  VolBox &operator=(const VolBox &src);

  // Get the number of points within this VolBox.
  unsigned int size();

  // OBS:  spheres should be a list of 4-membered Points:
  //       x, y, z, radius
  // If from_scratch is true, then bits are cleared and resized to
  // match self's number of points.
  void set_bits_for_spheres(const PointList &spheres, BitVector &bits,
                            bool from_scratch, unsigned int offset) const;

  void set_bits_for_one_sphere(const Point &sphere, BitVector &bits,
                               unsigned int offset) const;

  void get_points_within_spheres(const PointList &spheres,
                                 PointList &contained_points,
                                 unsigned int offset) const;

  // For folded fingerprints:
  void set_folded_bits_for_spheres(const PointList &spheres, BitVector &bits,
                                   unsigned int num_folds,
                                   unsigned int offset) const;

protected:
  float m_sphere_scale;
  float m_xmin, m_xmax, m_ymin, m_ymax, m_zmin, m_zmax;
  float m_units_per_side;
  float m_dx, m_dy, m_dz;
  int m_ixmax, m_iymax, m_izmax;
  XYZBucket *m_bucket;
  PointList m_bucket_points;

  void add_points(const PointList &points);
  void set_bits_for_one_sphere_unchecked(const Point &sphere, BitVector &bits,
                                         unsigned int offset) const;
  void set_folded_bits_for_one_sphere_unchecked(const Point &sphere,
                                                BitVector &bits,
                                                unsigned int offset,
                                                unsigned int folded_size) const;
  void get_points_in_cube(float x, float y, float z, float radius,
                          IndexList &result) const;
};
} // namespace shape
} // namespace mesaac
