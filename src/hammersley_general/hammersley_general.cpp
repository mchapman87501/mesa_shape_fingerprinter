// Hammersly Sequence generator.
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.
// Given dimension and number of points to return, return Hammersly points for
// that dimension.  The Hammersly Sequence of N dimensions is successive van der
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
// option for exactness.  This general program outputs numbers on the [0,1]
// interval for each dimension.  See the HammerslySpheroid program for confining
// that set of points to a 3 dimensional sphere or ellipsoid, centered on the
// origin with radius and eccentricity (a, b, c, where a = b = c for a sphere).

#include <bitset>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

using namespace std;
unsigned int p[28] = {3,  5,  7,  11, 13,  17,  19,  23, 29, 31,
                      37, 41, 43, 47, 53,  59,  61,  67, 71, 73,
                      79, 83, 89, 97, 101, 103, 107, 109};

vector<unsigned int> base_function(unsigned int prime, unsigned int number) {
  vector<unsigned int> base_vector;
  unsigned int quotient = number;

  while (quotient > 0) {
    base_vector.push_back(quotient % prime);
    quotient /= prime;
  }
  return base_vector;
};

void generate_points(const unsigned int dimension,
                     const unsigned int sample_size) {}

int main(int argc, char **argv) {

  unsigned int i, j, twocount;
  vector<unsigned int> primes(p, p + 28);
  vector<unsigned int> base_vector;
  unsigned int dimension;
  unsigned int sample_size;
  double sum;
  vector<float> a_dimension;
  vector<vector<float>> all_dimensions;

  // represent the integers i = 1,...,K as binary
  // multiple least significant bit by 1 plus its binary place reciprocal
  // e.g., random number x_i = 1111 = 1/16 + 1/8 + 1/4 + 1/2 = 0.9375
  // take the floor of (N * x_i) = random index into the array of length N
  // Store indices in a k length vector of ints.
  if (argc != 3) {
    const string prog_name = filesystem::path(argv[0]).filename().string();
    cerr << "Usage: " << prog_name << " dimension sample_size" << endl
         << "       dimension - the dimensionality of generated points," << endl
         << "                   typically some positive int less than 40."
         << endl
         << "       sample_size - the number of points to output." << endl;
    exit(0);
  }

  dimension = atoi(argv[1]);
  if (dimension > primes.size() + 1) {
    cerr << "Too many dimensions.  Current limit is " << primes.size() + 1
         << "." << endl
         << "Try again." << endl;
    exit(1);
  }

  sample_size = atoi(argv[2]);

  // Store first dimension, the sequence i/N (sample_size)
  for (i = 1; i <= sample_size; i++) {
    a_dimension.push_back((float)i / float(sample_size));
  }
  all_dimensions.push_back(a_dimension);
  a_dimension.clear();

  // van der Corput second dimension with 2 as the first prime.  Faster than
  // the remaining prime dimensions

  for (i = 1; i <= sample_size; i++) {
    j = 0;
    sum = 0.0;
    twocount = 1;
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

  all_dimensions.push_back(a_dimension);
  a_dimension.clear();

  // Remaining dimensions are successive primes 3, 5, 7, ...
  unsigned int count;
  unsigned int k = 0;
  while (k < dimension) {
    unsigned int prime = primes[k];
    for (i = 1; i <= sample_size; i++) {
      j = 0;
      sum = 0.0;
      count = 1;
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
    all_dimensions.push_back(a_dimension);
    a_dimension.clear();
    k++;
  }

  // Output points
  for (i = 0; i < sample_size; i++) {
    for (j = 0; j < dimension - 1; j++) {
      cout << all_dimensions[j][i] << " ";
    }
    cout << all_dimensions[j][i] << endl;
  }
}
