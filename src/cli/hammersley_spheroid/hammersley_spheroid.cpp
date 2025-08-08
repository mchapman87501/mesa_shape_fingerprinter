// Hammersly Sequence generator.
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.
// Given Dimension and number of points to return, return Hammersly points for
// that Dimension.  The Hammersly Sequence of N dimensions is successive van der
// Corput linear sequences, where the first dimension is the uniform sequence
// i/N, and where succeeding dimensions are van der Corput sequences generated
// with distinct prime bases.  The first dimension uses 2 as its prime number
// and is done in a separate for loop for performance.  The remaining dimensions
// make calls to the "radix_digits" to generate the van der Corput sequence for
// a specific prime base.  The benefits of quasi-random sequences of the
// Hammersly sequence begins to break down at around 40 dimensions without some
// fixes (not implemented here). Thus, the dimensions are capped at for now at
// 29, as this begins to bump up a floating point exception.  Floating point
// numbers are used for speed.  Double precision should be implemented as an
// option for exactness.

#include <array>
#include <bitset>
#include <iostream>
#include <vector>

#include "mesaac_arg_parser/arg_parser.hpp"

using namespace std;

using Point3D = array<float, 3>;
using PointVec = vector<Point3D>;

// In practice only the first of these primes is used.
static constexpr array<unsigned int, 29> primes{
    3,  5,  7,  11, 13, 17, 19, 23, 29, 31, 37,  41,  43,  47,
    53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109};

// Get the digits of a number n, base radix, in order from least significant to
// most significant.
vector<unsigned int> radix_digits(const unsigned int n,
                                  const unsigned int radix) {
  vector<unsigned int> base_vector;
  unsigned int quotient = n;

  while (quotient > 0) {
    base_vector.push_back(quotient % radix);
    quotient /= radix;
  }
  return base_vector;
};

void generate_points(const unsigned int sample_size, const float scale,
                     PointVec &result) {
  result.resize(sample_size);
  result.clear();

  // represent the integers i = 1,...,K as binary
  // multiply least significant bit by 1 plus its binary place reciprocal
  // e.g., random number x_i = 1111 = 1/16 + 1/8 + 1/4 + 1/2 = 0.9375
  // take the floor of (N * x_i) = random index into the array of length N
  // Store indices in a k length vector of ints.

  // Store first dimension, the sequence i/N (sample_size)
  size_t i_dim = 0;
  for (size_t i = 0; i < sample_size; i++) {
    result[i][i_dim] = (float)(i + 1) / float(sample_size);
  }

  // van der Corput second dimension with 2 as the first prime.  Faster than
  // the remaining prime dimensions
  i_dim += 1;
  for (size_t i = 1; i <= sample_size; i++) {
    size_t j = 0;
    double sum = 0.0;
    size_t twocount = 1;
    bitset<32> i_int(i);
    while (twocount <= i) {
      if (i_int[j]) {
        sum += 1.0 / (float)(twocount * 2);
      }
      j++;
      twocount *= 2;
    }
    result[i - 1][i_dim] = sum;
  }

  i_dim += 1;
  const unsigned int prime = primes[0];
  for (size_t i = 1; i <= sample_size; i++) {
    size_t j = 0;
    float sum = 0.0;
    size_t count = 1;
    const auto base_vector = radix_digits(i, prime);
    while (count <= i) {
      if (base_vector[j]) {
        sum += base_vector[j] / (float)(count * prime);
      }
      j++;
      count *= prime;
    }
    result[i - 1][i_dim] = sum;
  }

  // Center at origin (0,0,0) and scale
  for (unsigned int i = 0; i < sample_size; i++) {
    for (unsigned int j = 0; j < 3; j++) {
      result[i][j] = scale * (1.0 - 2.0 * result[i][j]);
    }
  }
}

void output_points(ostream &outs, const PointVec &points,
                   unsigned int sample_size, const float a, const float b,
                   const float c, const float scale) {
  const double scale_sqr = scale * scale;
  for (unsigned int i = 0; i < sample_size; i++) {
    const Point3D &point(points[i]);
    if (((point[0] * point[0]) / a + (point[1] * point[1]) / b +
         (point[2] * point[2]) / c) < scale_sqr) {
      outs << point[0] << " " << point[1] << " " << point[2] << endl;
    }
  }
}

int main(int argc, const char **const argv) {
  using namespace mesaac;

  auto sample_size = arg_parser::Argument<unsigned int>::create(
      "sample_size", "total number of 3D points to generate");
  auto a = arg_parser::Argument<float>::create("a", "ellipsoid x axis scale");
  auto b = arg_parser::Argument<float>::create("b", "ellipsoid y axis scale");
  auto c = arg_parser::Argument<float>::create("c", "ellipsoid z axis scale");
  auto scale = arg_parser::Argument<float>::create(
      "scale", "extent of largest ellipsoid axis");

  arg_parser::ArgParser args(
      {}, {sample_size, a, b, c, scale},
      "Generate sample_size points within a unit cube, and print those "
      "points\n"
      "that lie within an ellipsoid volume described by a, b, c, and scale.");
  int status = args.parse_args(argc, argv);
  if (status != 0 || args.usage_requested()) {
    return status;
  }

  // TODO extra cmdline arg validation.

  PointVec points;

  generate_points(sample_size->value(), scale->value(), points);

  output_points(cout, points, sample_size->value(), a->value(), b->value(),
                c->value(), scale->value());
  return 0;
}
