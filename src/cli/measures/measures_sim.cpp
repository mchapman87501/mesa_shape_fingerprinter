#include <cstdlib>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <sstream>
#include <string>
#include <vector>

#include "fingerprint_reader.hpp"
#include "measure_type_converter.hpp"
#include "mesaac_measures/shape_measures_factory.hpp"

using namespace std;

string Version = "1.2";
string CreationDate = "May, 2004";

void show_blurb() {
  cerr << "Running Measures." << endl
       << "Source code Copyright (c) 2002-2010 Mesa Analytics & Computing, Inc."
       << endl
       << "Version number " << Version << " Creation Date: " << CreationDate
       << endl;
}

void show_usage(int /* argc */, char **argv, const string msg = "") {
  cerr << "Usage: " << basename(argv[0])
       << " fingerprintfile.txt measure similarity format searchnumber | alpha "
          "| sparsethreshold"
       << endl
       << "measure = '-T' for Tanimoto, '-V' for Tversky," << endl
       << "'-E' for Euclidean, '-H' for Hamann, '-C' for Cosine, '-B' for BUB"
       << endl
       << "similarity = '-S' for similarity, '-D' for dissimilarity" << endl
       << "format = '-M' for Matrix, '-O' for Ordered Pairs, '-S' for Sparse "
          "Matrix"
       << endl
       << "searchnumber = an positive integer M < N to be searched" << endl
       << "Note: fingerprint file will have M + N fingerprints for searching = "
          "'-T'"
       << endl
       << "alpha is in the range 0-1, and measure must = '-V'" << endl
       << "sparsethreshold is in the range of (0,1), and format must be = '-S'"
       << endl;
  if (msg.size() > 0) {
    cerr << endl << msg << endl;
  }
  exit(1);
}

int main(int argc, char **argv) {
  using namespace mesaac::measures;
  using namespace mesaac::cli::measures;

  float tversky_alpha = 0.0;
  float sparse_threshold = 1.0;
  int signed_search_number;
  unsigned int search_number;

  show_blurb();

  if (argc != 6 && argc != 7 && argc != 8) {
    show_usage(argc, argv, "Wrong number of arguments");
  }

  // Input from either stdin or file
  string inputstring = argv[1];
  string distance_measure = argv[2];
  string similarity = argv[3];
  string format = argv[4];

  if (distance_measure.size() != 2) {
    ostringstream outs;
    outs << "Unknown measure '" << distance_measure << "'";
    show_usage(argc, argv, outs.str());
  }

  const auto measure_type =
      mesaac::cli::measures::get_measure_type(distance_measure[1]);

  const bool using_tversky = measure_type == MeasureType::tversky;

  if (similarity[1] != 'S' && similarity[1] != 'D') {
    cerr << "similarity option is not either S or D. Default to Dissimilarity"
         << endl;
  }

  if (using_tversky && (argc != 7 && argc != 8)) {
    show_usage(argc, argv);
  } else if (!using_tversky && format[1] != 'S' && (argc >= 7)) {
    show_usage(argc, argv, "Too many arguments.");
  }
  // Read in fingerprints from fingerprint file. First prime read to get vector
  // size
  signed_search_number = atoi(argv[5]);
  if (signed_search_number < 2) {
    ostringstream outs;
    outs << "Invalid search number " << argv[5] << ".  Value must be >= 2."
         << endl
         << "The search number tells which fingerprint is the first "
         << "'database' fingerprint.";
    show_usage(argc, argv, outs.str());
  }
  search_number = signed_search_number;

  if (using_tversky && format[1] != 'S' && argc == 7) {
    tversky_alpha = atof(argv[6]);
  } else if (using_tversky && format[1] == 'S' && argc == 8) {
    tversky_alpha = atof(argv[6]);
    sparse_threshold = atof(argv[7]);
  } else if (!using_tversky && format[1] == 'S' && argc == 7) {
    sparse_threshold = atof(argv[6]);
  } else if (argc != 6) {
    show_usage(argc, argv);
  }

  // Create a list of fingerprints to store each bitstring in
  mesaac::shape_defs::ArrayBitVectors fingerprints;
  read_fingerprints(inputstring, fingerprints);

  bool compute_sim = ('S' == similarity[1]);
  auto measure = get_measures(measure_type, tversky_alpha);
  auto measurer = mesaac::measures::shape::get_fp_measurer(measure, compute_sim,
                                                           fingerprints);
  if (0 == measurer) {
    cerr << "Could not create a shape fingerprint measure." << endl;
    exit(1);
  }

  const unsigned int number_fingerprints = fingerprints.size();
  unsigned int i, j;

  // TODO:  Abstract out the Tversky special-case output.
  if (format[1] == 'S') { // Sparse Matrix
    if (compute_sim) {
      for (i = 0; i < search_number; i++) {
        cout << i << "  ";
        for (j = search_number; j < number_fingerprints; j++) {
          float tmpmeasure = measurer->value(i, j);
          if (sparse_threshold <= tmpmeasure) {
            cout << j << " " << tmpmeasure << " ";
          }
        }
        cout << -1 << endl;
      }
      if (using_tversky) {
        // For Tversky, also output the complementary distances.
        for (i = 0; i < search_number; i++) {
          cout << i << "  ";
          for (j = search_number; j < number_fingerprints; j++) {
            float tmpmeasure = measurer->value(j, i);
            if (sparse_threshold <= tmpmeasure) {
              cout << j << " " << tmpmeasure << " ";
            }
          }
          cout << -1 << endl;
        }
      }
    } else {
      for (i = 0; i < search_number; i++) {
        cout << i << "  ";
        for (j = search_number; j < number_fingerprints; j++) {
          float tmpmeasure = measurer->value(i, j);
          if (sparse_threshold >= tmpmeasure) {
            cout << j << " " << tmpmeasure << " ";
          }
        }
        cout << -1 << endl;
      }
      if (using_tversky) {
        for (i = 0; i < search_number; i++) {
          cout << i << "  ";
          for (j = search_number; j < number_fingerprints; j++) {
            float tmpmeasure = measurer->value(j, i);
            if (sparse_threshold >= tmpmeasure) {
              cout << j << " " << tmpmeasure << " ";
            }
          }
          cout << -1 << endl;
        }
      }
    }
  } else if (format[1] == 'M') { // Matrix
    for (i = 0; i < search_number; i++) {
      for (j = search_number; j < number_fingerprints - 1; j++) {
        cout << measurer->value(i, j) << " ";
      }
      cout << measurer->value(i, j) << endl;
    }
    if (using_tversky) {
      for (i = 0; i < search_number; i++) {
        for (j = search_number; j < number_fingerprints - 1; j++) {
          cout << measurer->value(j, i) << " ";
        }
        cout << measurer->value(j, i) << endl;
      }
    }
  } else if (format[1] == 'O') { // Ordered Pair
    if (using_tversky) {
      for (i = 0; i < search_number; i++) {
        for (j = search_number; j < number_fingerprints; j++) {
          cout << i << " " << j << " " << measurer->value(i, j) << " "
               << measurer->value(j, i) << endl;
        }
      }

    } else {
      for (i = 0; i < search_number; i++) {
        for (j = search_number; j < number_fingerprints; j++) {
          cout << i << " " << j << " " << measurer->value(i, j) << endl;
        }
      }
    }
  }
  return 0;
}
