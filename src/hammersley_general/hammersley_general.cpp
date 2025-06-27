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
#include <iostream>
#include <string>
#include <vector>

using namespace std;

const vector<unsigned int> primes{3,  5,  7,  11, 13,  17,  19,  23, 29, 31,
                                  37, 41, 43, 47, 53,  59,  61,  67, 71, 73,
                                  79, 83, 89, 97, 101, 103, 107, 109};

bool str_to_unsigned_int(const string &sval, unsigned int &result,
                         const string &result_name) {
  // How to detect when sval is not even a valid integer, e.g., "foo"?
  int signed_val = atoi(sval.c_str());
  if (signed_val < 0) {
    cerr << "Value for " << result_name << " (" << sval
         << ") is not a valid unsigned integer." << endl;
    return false;
  }
  result = signed_val;
  return true;
}

void show_usage(const string &prog_name) {
  cerr << "Usage: " << prog_name << " (-h|--help) dimension sample_size" << endl
       << "    dimension - the dimensionality of generated points," << endl
       << "                typically some positive int less than 40." << endl
       << "    sample_size - the number of points to output." << endl
       << endl
       << "Options:" << endl
       << "    -h|--help - show this help message and exit" << endl;
}

void parse_cmdline(int argc, char **argv, unsigned int &dimension,
                   unsigned int &sample_size) {
  const string prog_name = filesystem::path(argv[0]).filename().string();

  if (argc >= 2 && (string(argv[1]) == "-h" || string(argv[1]) == "--help")) {
    show_usage(prog_name);
    exit(0);
  }

  if (argc != 3) {
    show_usage(prog_name);
    exit(1);
  }

  if (!str_to_unsigned_int(argv[1], dimension, "dimension")) {
    exit(1);
  }
  if (dimension < 1) {
    cerr << "dimension (" << dimension << ") must be greater than zero."
         << endl;
    exit(1);
  }

  if (dimension > primes.size() + 1) {
    cerr << "dimension (" << dimension << ") must be less than or equal to "
         << primes.size() + 1 << "." << endl;
    exit(1);
  }

  if (!str_to_unsigned_int(argv[2], sample_size, "sample_size")) {
    exit(1);
  }
  if (sample_size < 0) {
    cerr << "sample_size (" << sample_size << ") must be non-negative" << endl;
    exit(1);
  }
}

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
                     const unsigned int sample_size,
                     vector<vector<float>> &result) {
  vector<float> a_dimension;

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
  unsigned int count;
  unsigned int k = 0;
  while (k < dimension) {
    unsigned int prime = primes[k];
    for (unsigned int i = 1; i <= sample_size; i++) {
      unsigned int j = 0;
      double sum = 0.0;
      count = 1;
      vector<unsigned int> base_vector = base_function(prime, i);
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
}

int main(int argc, char **argv) {

  unsigned int dimension;
  unsigned int sample_size;
  vector<vector<float>> all_dimensions;

  // represent the integers i = 1,...,K as binary
  // multiple least significant bit by 1 plus its binary place reciprocal
  // e.g., random number x_i = 1111 = 1/16 + 1/8 + 1/4 + 1/2 = 0.9375
  // take the floor of (N * x_i) = random index into the array of length N
  // Store indices in a k length vector of ints.

  parse_cmdline(argc, argv, dimension, sample_size);
  generate_points(dimension, sample_size, all_dimensions);

  // Output points
  for (unsigned int i = 0; i < sample_size; i++) {
    string sep("");
    for (unsigned int j = 0; j < dimension; j++) {
      cout << sep << all_dimensions[j][i];
      sep = " ";
    }
    cout << endl;
  }
}
