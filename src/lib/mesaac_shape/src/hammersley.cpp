//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_shape/hammersley.hpp"

#include <bitset>
#include <iostream>
#include <numbers>

namespace mesaac::shape {
using namespace std;

namespace {
// In practice only the first of these primes is used.
static constexpr array<unsigned int, 29> primes{
    3,  5,  7,  11, 13, 17, 19, 23, 29, 31, 37,  41,  43,  47,
    53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109};

using DigitsVector = vector<int>;

DigitsVector radix_digits(int n, int radix) {
  DigitsVector result;
  while (n) {
    result.push_back(n % radix);
    n = n / radix;
  }
  return result;
}

float hamm_dim_0(unsigned int index, unsigned int sample_size) {
  // the first dimension is just the sequence i/N (sample_size)
  return float(index) / float(sample_size);
}

// Subsequent dimensions D in 1...N are mathematically the same (van der
// Corput dimension computed from a prime).  Here, a separate implementation is
// used for dimension 1, i.e., for prime 2, to take advantage of C++ base-2
// facilities.

float hamm_dim_1(const unsigned int index) {
  size_t j = 0;
  float result = 0.0;
  size_t twocount = 1;
  bitset<32> i_int(index);
  while (twocount <= index) {
    if (i_int[j]) {
      result += 1.0 / (float)(twocount * 2);
    }
    j++;
    twocount *= 2;
  }
  return result;
}

float hamm_dim_n(const unsigned int dimension, const unsigned int index) {
  const auto prime = primes.at(dimension - 2);
  size_t j = 0;
  float result = 0.0;
  size_t count = 1;
  const auto digits = radix_digits(index, prime);
  while (count <= index) {
    if (digits[j]) {
      result += digits[j] / (float)(count * prime);
    }
    j++;
    count *= prime;
  }
  return result;
}
} // namespace

Point3D Hammersley::next_point() {
  // Get the next hammersley point that lies within the spheroid.
  for (;;) {
    m_point_index += 1;
    if (m_point_index > m_num_points) {
      return {0, 0, 0};
    }
    return {
        hamm_dim_0(m_point_index, m_num_points),
        hamm_dim_1(m_point_index),
        hamm_dim_n(2, m_point_index),
    };
  }
}

void Hammersley::get_ellipsoid(const Hammersley::EllipsoidParams &params,
                               Point3DList &result) {
  const Point3D zero{0, 0, 0};

  const float scale_2 = params.scale * 2.0;
  const float scale_sqr = params.scale * params.scale;

  // Estimate the fraction of unit volume covered by a spheroid.
  // From Wikipedia (https://en.wikipedia.org/wiki/Ellipsoid#Volume)
  // the volume of an ellipsoid with scaling factors a, b, c --
  // where a^2 corresponds to params.a in this code, b^2 to params.b,
  // c^2 to params.c --
  // is V = 4π/3 a b c
  // Suppose a, b and c are expressed as fractions of the unit radius r.
  // Then the corresponding cube has volume 8 r r r
  // and the ellipsoid volume as a fraction of the unit cube
  // volume is π/6 a b c.
  //
  const float fraction = ::sqrt(params.a) * ::sqrt(params.b) *
                         ::sqrt(params.c) * std::numbers::pi / 6;
  size_t total_points = params.num_points / fraction;
  if (total_points * fraction < params.num_points) {
    total_points += 1;
  }
  Hammersley gen(total_points);
  result.clear();
  result.reserve(params.num_points);
  for (size_t num_generated = 0;; ++num_generated) {
    if (result.size() >= params.num_points) {
      // Enough points
      std::cerr << "DBG: get_ellipsoid filled #points = " << params.num_points
                << " after generating #points = " << num_generated << " of "
                << total_points << " or "
                << (int)(100 * num_generated / total_points)
                << "% of all estimated." << std::endl;
      return;
    }

    const Point3D raw_point = gen.next_point();
    if (raw_point == zero) {
      // No more points available
      std::cerr << "DBG: get_ellipsoid ran out of points after filling "
                   "#points = "
                << result.size() << " of required " << params.num_points
                << ", or " << (int)(100 * result.size() / params.num_points)
                << "%" << std::endl;
      return;
    }

    // Scale & mean-center the point.
    const float x = params.scale - scale_2 * raw_point[0],
                y = params.scale - scale_2 * raw_point[1],
                z = params.scale - scale_2 * raw_point[2];
    // Filter points that lie outside the spheroid.
    const float xsqr = x * x / params.a;
    const float ysqr = y * y / params.b;
    const float zsqr = z * z / params.c;
    if ((xsqr + ysqr + zsqr) < scale_sqr) {
      result.emplace_back(Point3D{x, y, z});
    }
  }
}

void Hammersley::get_cuboid(const CuboidParams &params, Point3DList &result) {
  const Point3D zero{0, 0, 0};
  result.clear();
  result.reserve(params.num_points);

  // Real-world bounds
  const float dxw = params.xmax - params.xmin, dyw = params.ymax - params.ymin,
              dzw = params.zmax - params.zmin;
  const float dw_max = max(max(dxw, dyw), dzw);

  if (dw_max > 0) {
    // Dimensions of the unit subcube, which is centered at 0,0,0.
    const float dx = dxw / dw_max, dy = dyw / dw_max, dz = dzw / dw_max;
    // Fraction of unit volume covered by subcube:
    const float fract = dx * dy * dz;
    if (0 < fract && fract <= 1.0) {
      // How many unit cube points must be generated in order
      // to get num_points within the subvolume?
      unsigned int total_points = params.num_points / fract;
      if (total_points * fract < params.num_points) {
        total_points += 1;
      }

      Hammersley gen(total_points);

      for (;;) {
        if (result.size() >= params.num_points) {
          // Enough points
          return;
        }

        const Point3D raw_point = gen.next_point();
        if (raw_point == zero) {
          // No more points available
          return;
        }

        const float x(raw_point[0]), y(raw_point[1]), z(raw_point[2]);
        if ((x <= dx) && (y <= dy) && (z <= dz)) {
          // Shift points to center.
          result.emplace_back(Point3D{(x * dw_max) + params.xmin,
                                      (y * dw_max) + params.ymin,
                                      (z * dw_max) + params.zmin});
        }
      }
    }
  }
}
} // namespace mesaac::shape
