//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape/hammersley.hpp"

using namespace std;

namespace mesaac::shape {

typedef vector<int> DigitsVector;

Hammersley::Hammersley() {
  m_num_points = 0;
  m_num_generated = 0;
}

void Hammersley::get_cubic(float xmin, float xmax, float ymin, float ymax,
                           float zmin, float zmax, unsigned int num_points,
                           PointList &result) {
  result.clear();
  result.reserve(num_points);

  // Real-world bounds
  float dxw = xmax - xmin, dyw = ymax - ymin, dzw = zmax - zmin;
  float dw_max = dxw;
  dw_max = (dw_max > dyw) ? dw_max : dyw;
  dw_max = (dw_max > dzw) ? dw_max : dzw;

  if (dw_max > 0) {
    // Dimensions of the unit subcube, which is centered at 0,0,0.
    float dx = dxw / dw_max, dy = dyw / dw_max, dz = dzw / dw_max;
    // Fraction of unit volume covered by subcube:
    float fract = dx * dy * dz;
    if (0 < fract && fract <= 1.0) {
      // How many unit cube points must be generated in order
      // to get num_points within the subvolume?
      unsigned int total_points = num_points / fract;
      if (total_points * fract < num_points) {
        total_points += 1;
      }

      Hammersley h;
      Point p;
      h.start(total_points);
      while (result.size() != num_points && h.next_point(p)) {
        float x(p[0]), y(p[1]), z(p[2]);
        if ((x <= dx) && (y <= dy) && (z <= dz)) {
          // Shift points to center.
          p[0] = (x * dw_max) + xmin;
          p[1] = (y * dw_max) + ymin;
          p[2] = (z * dw_max) + zmin;
          result.push_back(p);
        }
      }
    }
  }
}

void Hammersley::start(unsigned int num_points) {
  m_num_points = num_points;
  m_num_generated = 0;
}

static inline void get_radix_digits(int n, int radix, DigitsVector &result) {
  result.clear();
  while (n) {
    result.push_back(n % radix);
    n = n / radix;
  }
}

static const int num_dimensions = 3;
static const float primes[num_dimensions - 1] = {2, 3};

bool Hammersley::next_point(Point &pnt) {
  if (m_num_generated >= m_num_points) {
    pnt.clear();
    pnt.resize(num_dimensions, 0.0);
    return false;
  }

  m_num_generated++;
  pnt.clear();
  pnt.reserve(num_dimensions);
  pnt.push_back(m_num_generated / float(m_num_points));
  for (int k = 0; k < num_dimensions - 1; ++k) {
    float prime = primes[k];
    float count = 1.0;
    float total = 0.0;
    DigitsVector rd;
    get_radix_digits(m_num_generated, prime, rd);
    DigitsVector::iterator di;
    for (di = rd.begin(); di != rd.end(); ++di) {
      int digit(*di);
      count *= prime;
      total += digit / count;
    }
    pnt.push_back(total);
  }
  return true;
}
} // namespace mesaac::shape
