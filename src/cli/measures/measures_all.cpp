#include <cstdlib>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "Globals.h"
#include "fingerprint_reader.h"
#include "mesaac_measures/measurer_factory.h"

#include "mesaac_common/mesaac_common.h"

using namespace std;

string Version = "1.2";
string CreationDate = "October 17, 2005";

void show_blurb() {
  cerr << "Running MeasuresAll " << endl
       << "Source code Copyright (c) 2007-2010" << endl
       << "Mesa Analytics & Computing, Inc." << endl
       << "Version number " << Version << " Creation Date: " << CreationDate
       << endl;
}

void show_usage(int /* argc */, char **argv, string msg = "") {
  cerr << "Usage: " << basename(argv[0])
       << " fingerprintfile.txt measure similarity format searching" << endl
       << "       searchnumber | alpha | sparsethreshold" << endl
       << "measure      = '-T' for Tanimoto, '-V' for Tversky, '-E' for "
          "Euclidean,"
       << endl
       << "               '-H' for Hamann, '-C' for Cosine, '-B' for BUB"
       << endl
       << "similarity   = '-S' for similarity, '-D' for dissimilarity" << endl
       << "format       = '-M' for Matrix, '-O' for Ordered Pairs, " << endl
       << "               '-S' for Sparse Matrix, '-P' for PVM" << endl
       << "searching    = '-T' for similarity searching, '-F' if not" << endl
       << "searchnumber = a positive integer M < N to be searched" << endl
       << endl
       << "Note: fingerprint file will have M + N fingerprints for searching = "
          "'-T'"
       << endl
       << "alpha is in the range 0-1, and measure must = '-V'" << endl
       << "sparsethreshold is in the range of (0,1), and format must be = '-S' "
          "or '-P'"
       << endl;
  if (msg.size() > 0) {
    cerr << endl << msg << endl;
  }
  exit(1);
}

int main(int argc, char **argv) {
  unsigned int i, j;
  float tversky_alpha = 0;
  float sparse_threshold = 0;
  unsigned int search_number = 0;

  mesaac::MesaacFeatures f;
  f.set(mesaac::MFE_GroupingModule);
  mesaac::initCommon(f);

  show_blurb();

  if ((argc < 7) || (argc > 9)) {
    show_usage(argc, argv, "Wrong number of arguments");
  }

  string distance_measure = argv[2];
  if (distance_measure.size() != 2) {
    ostringstream outs;
    outs << "Unknown measure '" << distance_measure << "'";
    show_usage(argc, argv, outs.str());
  }

  string similarity = argv[3];
  string format = argv[4];
  string searching = argv[5];

  if (similarity[1] != 'S' && similarity[1] != 'D') {
    cerr << "similarity option is not either S or D. Default to Dissimilarity"
         << endl;
  }

  if (distance_measure[1] == 'V' && (argc != 8 && argc != 9)) {
    show_usage(argc, argv, "No alpha value supplied for Tversky measure.");
  } else if (distance_measure[1] != 'V' && format[1] != 'S' &&
             format[1] != 'P' && (argc == 8 || argc == 9)) {
    show_usage(argc, argv, "Too many arguments");
  }

  // Format "-P" implies searching:
  if ((searching[1] == 'T') || (format[1] == 'P')) {
    search_number = atoi(argv[6]);
  }
  if (distance_measure[1] == 'V' && format[1] != 'S' && format[1] != 'P' &&
      argc == 8) {
    tversky_alpha = atof(argv[7]);
  } else if (distance_measure[1] == 'V' &&
             (format[1] == 'S' || format[1] == 'P') && argc == 9) {
    tversky_alpha = atof(argv[7]);
    sparse_threshold = atof(argv[8]);
  } else if (distance_measure[1] != 'V' &&
             (format[1] == 'S' || format[1] == 'P') && argc == 8) {
    sparse_threshold = atof(argv[7]);
  } else if (argc != 7) {
    show_usage(argc, argv, "Wrong number of arguments");
  }

  bool compute_sim = ('S' == similarity[1]);
  const char dist_type(distance_measure[1]);

  // cerr << "Settings:" << endl
  //      << "    Measure: " << dist_type << endl
  //      << "    Similarity? " << compute_sim << endl
  //      << "    Format: " << format << endl
  //      << "    Searching: " << searching << " search number, if applicable: "
  //      << search_number << endl
  //      << "    Tversky alpha, if applicable: " << tversky_alpha << endl
  //      << "    Sparse threshold, if app.:    " << sparse_threshold << endl
  //      ;

  // Create a list of fingerprints to store each bitstring in
  ArrayBitVectors fingerprints;
  read_fingerprints(argv[1], fingerprints);

  IMeasurer *measurer =
      get_measurer(compute_sim, dist_type, fingerprints, tversky_alpha);
  if (0 == measurer) {
    ostringstream msg;
    msg << "Unknown measure -" << dist_type
        << ".  Please specify one of  -T, -V, -E, -H or -C";
    show_usage(argc, argv, msg.str());
  }

  const unsigned int number_fingerprints = fingerprints.size();

  if ((searching == "-T") &&
      ((search_number <= 0) || (search_number >= number_fingerprints))) {
    ostringstream msg;
    msg << "Search number (" << search_number << ") must be in the range 1.."
        << number_fingerprints << " (number of fingerprints), inclusive.";
    show_usage(argc, argv, msg.str());
  }
  if (format[1] == 'O') { // ordered pair format
    for (i = 0; i < number_fingerprints; i++) {
      for (j = 0; j < number_fingerprints; j++) {
        cout << i << " " << j << " " << measurer->value(i, j) << endl;
      }
    }
  } else if (format[1] == 'M') { // Matrix format
    for (i = 0; i < number_fingerprints; i++) {
      string sep("");
      for (j = 0; j < number_fingerprints; j++) {
        cout << sep << measurer->value(i, j);
        sep = " ";
      }
      cout << endl;
    }
  } else if (format[1] == 'S') { // Sparse Matrix
    unsigned int i_end = number_fingerprints;
    unsigned int j_start = 0;
    bool is_searching = (searching == "-T");
    if (is_searching) {
      i_end = search_number;
      j_start = search_number;
    }
    if (compute_sim) {
      for (i = 0; i < i_end; i++) {
        if (is_searching) {
          cout << i << " ";
        }
        for (j = j_start; j < number_fingerprints; j++) {
          if (i != j) {
            float v = measurer->value(i, j);
            if (sparse_threshold <= v) {
              cout << j << " " << v << " ";
            }
          }
        }
        cout << -1 << endl;
      }
    } else {
      for (i = 0; i < i_end; i++) {
        if (is_searching) {
          cout << i << " ";
        }
        for (j = j_start; j < number_fingerprints; j++) {
          if (i != j) {
            float v = measurer->value(i, j);
            if (sparse_threshold >= v) {
              cout << j << " " << v << " ";
            }
          }
        }
        cout << -1 << endl;
      }
    }
  } else if (format[1] == 'P') { // PVM
    if (compute_sim) {
      for (i = 0; i < search_number; i++) {
        for (j = search_number; j < number_fingerprints; j++) {
          float v = measurer->value(i, j);
          if (sparse_threshold <= v) {
            cout << (j - search_number) << " " << v << " ";
          }
        }
        cout << -1 << endl;
      }
    } else {
      for (i = 0; i < search_number; i++) {
        for (j = search_number; j < number_fingerprints; j++) {
          float v = measurer->value(i, j);
          if (sparse_threshold >= v) {
            cout << (j - search_number) << " " << v << " ";
          }
        }
        cout << -1 << endl;
      }
    }
  }
  return 0;
}
