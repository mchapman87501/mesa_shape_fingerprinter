// Hammersly Sequence generator.
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.
// Given Dimension and number of points to return, return Hammersly points for
// that Dimension.  The Hammersly Sequence of N dimensions is successive van der
// Corput linear sequences, where the first dimension is the uniform sequence
// i/N, and where succeeding dimensions are van der Corput sequences generated
// with distinct prime bases.  The first dimension uses 2 as its prime number
// and is done in a separate for loop for performance.  The remaining dimensions
// make calls to the "base_function" to generate the van der Corput sequence for
// a specific prime base.  The benefits of quasi-random sequences of the
// Hammersly sequence begins to break down at around 40 dimensions without some
// fixes (not implemented here). Thus, the dimensions are capped at for now at
// 29, as this begins to bump up a floating point exception.  Floating point
// numbers are used for speed.  Double precision should be implemented as an
// option for exactness.

#include <array>
#include <bitset>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <string>
#include <vector>

#include "mesaac_arg_parser/arg_parser.hpp"

using namespace std;
static constexpr array<unsigned int, 29> primes{
    3,  5,  7,  11, 13, 17, 19, 23, 29, 31, 37,  41,  43,  47,
    53, 59, 61, 67, 71, 73, 79, 83, 89, 97, 101, 103, 107, 109};

vector<unsigned int> base_function(unsigned int prime, unsigned int number) {
  vector<unsigned int> base_vector;
  unsigned int quotient = number;

  while (quotient > 0) {
    base_vector.push_back(quotient % prime);
    quotient /= prime;
  }
  return base_vector;
};

void generate_points(const unsigned int sample_size, const float scale,
                     vector<vector<float>> &result) {
  vector<unsigned int> base_vector;
  vector<float> a_dimension;

  // represent the integers i = 1,...,K as binary
  // multiply least significant bit by 1 plus its binary place reciprocal
  // e.g., random number x_i = 1111 = 1/16 + 1/8 + 1/4 + 1/2 = 0.9375
  // take the floor of (N * x_i) = random index into the array of length N
  // Store indices in a k length vector of ints.

  // Store first dimension, the sequence i/N (sample_size)
  for (unsigned int i = 1; i <= sample_size; i++) {
    a_dimension.push_back((float)i / float(sample_size));
  }
  result.push_back(a_dimension);
  a_dimension.clear();

  // van der Corput second dimension with 2 as the first prime.  Faster than
  // the remaining prime dimensions

  for (unsigned int i = 1; i <= sample_size; i++) {
    unsigned int j = 0;
    double sum = 0.0;
    unsigned int twocount = 1;
    bitset<32> i_int(i);
    while (twocount <= i) {
      if (i_int[j]) {
        sum += 1.0 / (float)(twocount * 2);
      }
      j++;
      twocount *= 2;
    }
    a_dimension.push_back(sum);
  }

  result.push_back(a_dimension);
  a_dimension.clear();

  // Remaining dimensions are successive primes 3, 5, 7, ...
  unsigned int k = 0;
  while (k < 3) {
    unsigned int prime = primes[k];
    for (unsigned int i = 1; i <= sample_size; i++) {
      unsigned int j = 0;
      double sum = 0.0;
      unsigned int count = 1;
      base_vector = base_function(prime, i);
      while (count <= i) {
        if (base_vector[j]) {
          sum += base_vector[j] / (float)(count * prime);
        }
        j++;
        count *= prime;
      }
      a_dimension.push_back(sum);
    }
    result.push_back(a_dimension);
    a_dimension.clear();
    k++;
  }

  // Center at origin (0,0,0) and scale
  for (unsigned int i = 0; i < sample_size; i++) {
    for (unsigned int j = 0; j < 3; j++) {
      result[j][i] = scale * (1.0 - 2.0 * result[j][i]);
    }
  }
}

void output_points(ostream &outs, const vector<vector<float>> &points,
                   unsigned int sample_size, const float a, const float b,
                   const float c, const float scale) {
  const double scale_sqr = scale * scale;
  for (unsigned int i = 0; i < sample_size; i++) {
    if ((((points[0][i]) * (points[0][i])) / a +
         ((points[1][i]) * (points[1][i])) / b +
         ((points[2][i]) * (points[2][i])) / c) < scale_sqr) {
      outs << points[0][i] << " " << points[1][i] << " " << points[2][i]
           << endl;
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

  unsigned int i, j, twocount;
  vector<vector<float>> points;

  generate_points(sample_size->value(), scale->value(), points);

  output_points(cout, points, sample_size->value(), a->value(), b->value(),
                c->value(), scale->value());
  return 0;
}
