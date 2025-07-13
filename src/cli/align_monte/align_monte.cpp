// Shape Alignment using quasi-Monte Carlo sampling and PCA
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.
//
// This program takes as input an sd file of conformers, a file of Hammersly
// points arrayed in a sphere, centered about the origin, of a size into
// which small molecules will fit, in an orientation whereby the small
// molecules are aligned to the first molecule in the sd file.

// Step
// 1.  Mean center atom centers of the first conformer ("reference conformer").
//     a. find the Hammersly points in the sphere that are inside the
//        atom radii of this conformer
//     b. compute the principle components of the point set
//     c. rotate the atom centers to the new principle components' axes
//        such that x, y, and z are the first, second, and third principle
//        components respectively.
//     d. Calculate this conformer's fingerprint in this 1 position
//
// 2.  For each of the remaining conformers repeat step 1, except 1) in
//     substep d calculate all 4 fingerprints of the conformer (rotating the
//     the conformer about the 3 major axes, 180 degree rotations), and then
//     calculate the maximum similarity of each of these 4 fingerprints
//     with the fingerprint of the first conformer in the sd input file
//     (step 1).  2) Mean center each conformer to the mean center of the
//     of the first conformer in step 1. 3) output the conformer in the
//     position found via the maximum similarity of fingerprints.
//
// 3.  The output is thus an .sdf file containing all of the molecules aligned
//     to the first molecule, and the first molecule is aligned to its
//     principle components about (0,0,0) origin, and the remaining molecules
//     are along their principle components and about 0,0,0, but aligned to
//     the first molecule.
//
// Code:
//
// 1. Read in the first conformer from the conformer file.
//    a.  Align this conformer to its Principle Component axes.
//    b.  Create one fingerprint with hammersly sphere of quasi-random points.
//    c.  Output conformer in the new coordinates, with Tanimoto tag of 1.
//
// 2. Read in next conformer from file.
//    a.  Align this conformer to its Principle Component axes.
//    b.  Create eight fingerprints with same hammersly sphere of quasi-random
//        points.
//    c.  Compare 4 fingerprints to the fingerprint of the first conformer to
//        find max Tanimoto value
//    d.  Output conformer in the new coordinates, respective of max Tanimoto,
//        with the max Tanimoto tag.
//

#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include <unistd.h>

#include "mesaac_shape/shared_types.hpp" // For bitvector types
#include "sdf_mol_aligner.hpp"
#include "shared_types.hpp"

// Measures classes:
#include "mesaac_measures/measures_base.hpp"

#include "mesaac_measures/bub.hpp"
#include "mesaac_measures/cosine.hpp"
#include "mesaac_measures/euclidean.hpp"
#include "mesaac_measures/tanimoto.hpp"
#include "mesaac_measures/tversky.hpp"

using namespace std;
namespace mesaac::align_monte {

namespace {
string lowercase(const string s) {
  string result(s);
  transform(s.begin(), s.end(), result.begin(), ::tolower);
  return result;
}

typedef enum {
  // Enum values can be bitwise ORed to create masks
  MIE_Invalid = 0,
  MIE_BUB = 1,
  MIE_Tani = 2,
  MIE_Cosine = 4,
  MIE_Tversky = 8
} MeasureIDEnum;

typedef vector<MeasureIDEnum> MeasureIDList;

MeasureIDEnum get_measure_id(const string measure_name) {
  const string lcname(lowercase(measure_name));
  if (lcname == "b") {
    return MIE_BUB;
  } else if (lcname == "t") {
    return MIE_Tani;
  } else if (lcname == "c") {
    return MIE_Cosine;
  } else if (lcname == "v") {
    return MIE_Tversky;
  }
  return MIE_Invalid;
}

string get_measure_name(MeasureIDEnum id) {
  string result = "unknown measure";
  switch (id) {
  case MIE_BUB:
    result = "BUB";
    break;

  case MIE_Tani:
    result = "Tanimoto";
    break;

  case MIE_Cosine:
    result = "Cosine";
    break;

  case MIE_Tversky:
    result = "Tversky";
    break;

  default:
    result = "*invalid measure*";
    break;
  }
  return result;
}

void get_measures(MeasureIDList &ids, float tversky_alpha,
                  MeasuresList &result) {
  result.clear();
  for (const auto &measure_id : ids) {
    switch (measure_id) {
    case MIE_BUB:
      result.push_back(std::make_shared<measures::BUB>());
      break;

    case MIE_Tani:
      result.push_back(std::make_shared<measures::Tanimoto>());
      break;

    case MIE_Cosine:
      result.push_back(std::make_shared<measures::Cosine>());
      break;

    case MIE_Tversky:
      result.push_back(std::make_shared<measures::Tversky>(tversky_alpha));
      break;

    case MIE_Invalid:
      // This should have been handled during command-line parsing.
      cerr << "Internal error: invalid measure specified" << endl;
      break;
    }
  }
}

class ArgParser {
public:
  ArgParser(int argc, char **argv) : m_argc(argc), m_argv(argv) {}

  void get_args(string &sd_pathname, string &hs_pathname, float &atom_scale,
                bool &atom_centers_only, MeasureIDList &measure_ids,
                float &tversky_alpha, string &sorted_pathname) {
    atom_centers_only = false;
    string measure_name("");
    MeasureIDEnum measures_applied = MIE_Invalid;
    bool duplicated_measures = false, duplicated_tversky = false;
    measure_ids.clear();
    tversky_alpha = 0.0;
    sorted_pathname = "";

    int i = 1;
    while (i < m_argc) {
      string curr_arg(m_argv[i]);
      if (curr_arg.substr(0, 1) != "-") {
        break;
      }
      if ((curr_arg == "-a") || (curr_arg == "--atom-centers")) {
        atom_centers_only = true;
      } else if ((curr_arg == "-s") || (curr_arg == "--sort")) {
        get_value_for("SORT_FILE", i, sorted_pathname);
      } else if ((curr_arg == "-m") || (curr_arg == "--measure")) {
        get_value_for("MEASURE", i, measure_name);
        // Add this measure type, if it has not already been added.
        const MeasureIDEnum curr_id = get_measure_id(measure_name);
        if (curr_id == MIE_Invalid) {
          show_usage("Unknown measure '" + measure_name + "'");
        }
        if (0 == (curr_id & measures_applied)) {
          measure_ids.push_back(curr_id);
          measures_applied = (MeasureIDEnum)(measures_applied | curr_id);
        } else {
          duplicated_measures = true;
          if (MIE_Tversky == curr_id) {
            duplicated_tversky = true;
          }
        }
        // If Tversky is specified multiple times, the last
        // specified Tversky alpha wins.
        if (MIE_Tversky == curr_id) {
          string alpha_str;
          get_value_for("ALPHA", i, alpha_str);
          istringstream ins(alpha_str);
          if (!(ins >> tversky_alpha)) {
            ostringstream msg;
            msg << "Could not convert ALPHA value '" << m_argv[i]
                << "' to a number.";
            show_usage(msg);
          }
          if ((0 > tversky_alpha) || (2 < tversky_alpha)) {
            show_usage("ALPHA value must be in the range 0..2 inclusive");
          }
        }
      } else if ((curr_arg == "-h") || (curr_arg == "--help")) {
        show_usage("", 0);
      } else {
        ostringstream msg;
        msg << "Unsupported option '" << curr_arg << "'";
        show_usage(msg);
      }
      i++;
    }

    if (measure_ids.empty()) {
      // Default:
      measure_ids.push_back(MIE_Tani);
    }

    if ((m_argc - i) != 3) {
      show_usage("Wrong number of arguments.");
    }
    sd_pathname = m_argv[i++];
    hs_pathname = m_argv[i++];

    string atom_scale_str(m_argv[i++]);
    istringstream ins(atom_scale_str);
    ins >> atom_scale;
    if (!ins) {
      ostringstream err;
      err << "Value for atom_scale ('" << atom_scale_str
          << "') is not a float.";
      show_usage(err);
    }
    if ((atom_scale < 1.0) || (atom_scale > 2.0)) {
      ostringstream err;
      err << "atom_scale must be in the range [1.0 .. 2.0]";
      show_usage(err);
    }

    if (duplicated_measures) {
      cerr << "Some measures were specified multiple times." << endl
           << "Conformers will be flipped for best alignment "
           << "according to this sequence of measures:" << endl;
      string sep = "";
      for (const auto &measure_id : measure_ids) {
        cerr << sep << get_measure_name(measure_id);
        sep = ", ";
      }
      cerr << endl;
    }

    if (duplicated_tversky) {
      cerr << "Using last-specified Tversky alpha " << tversky_alpha << endl;
    }
  }

  void show_usage(string err_msg, int status = 1) {
    cerr << "Usage: " << m_argv[0]
         << " [options] sd_file hamms_sphere_file atom_scale" << endl
         << "Align all conformers in an SD file." << endl
         << endl
         << "sd_file           = a file of conformers in SD format, with 3D "
            "coordinates"
         << endl
         << "hamms_sphere_file = a file containing 3D Hammersley sphere "
            "points, one point"
         << endl
         << "                    per line with space-separated coordinates, "
            "for principal"
         << endl
         << "                    axes generation via SVD" << endl
         << "atom_scale        = the amount, in the range [1.0..2.0], by which "
            "to "
         << endl
         << "                    increase atom radii for alignment" << endl
         << endl
         << "Options: " << endl
         << "-a|--atom-centers = if specified, perform alignment using atom "
            "centers only;"
         << endl
         << "                    otherwise use contained Hammersley sphere "
            "points to"
         << endl
         << "                    perform alignment." << endl
         << "[-m|--measure B | T | C | V ALPHA [-m|--measure ...]]" << endl
         << "                  = the measures to use in finding the best "
            "alignments"
         << endl
         << "                    (B=BUB, T=Tanimoto, C=Cosine, V=Tversky)"
         << endl
         << "                    Note:  If you specify Tversky ('-m V', you "
            "must also "
         << endl
         << "                    provide an ALPHA value in the range 0..2 "
            "inclusive."
         << endl
         << "                    If no measure option is specified, Tanimoto "
            "will be used"
         << endl
         << "                    For each specified measure type, the SDF "
            "conformers will"
         << endl
         << "                    be tagged with corresponding "
            "<MaxAlign[MEASURE NAME]> and"
         << endl
         << "                    <BestFlip[MEASURE NAME]> properties" << endl
         << "-s|--sort SORT_FILE" << endl
         << "                  = Create the named SORT_FILE containing SDF "
            "record indices "
         << endl
         << "                    and corresponding measure values, in "
            "descending value"
         << endl
         << "                    order.  The last specified measure will be "
            "used as the sort"
         << endl
         << "                    value." << endl
         << "-h | --help       = print this help message and exit" << endl;

    if (err_msg.size()) {
      cerr << endl << err_msg << endl;
    }
    exit(status);
  }
  void show_usage(const char *err_msg) { show_usage(string(err_msg)); }
  void show_usage(ostringstream &err_msg) { show_usage(err_msg.str()); }

protected:
  int m_argc;
  char **m_argv;

  void get_value_for(string option_name, int &i, string &sval) {
    i++;
    if (i >= m_argc) {
      ostringstream msg;
      msg << "No value specified for " << option_name;
      show_usage(msg);
    } else {
      sval = m_argv[i];
    }
  }
};
} // namespace
} // namespace mesaac::align_monte

int main(int argc, char **argv) {
  using namespace mesaac::align_monte;

  string sd_pathname("");
  string hamms_sphere_pathname("");
  float atom_scale = 1.0;
  bool atom_centers_only = false;
  MeasureIDList measure_ids;
  float tversky_alpha = 0.0;
  string sorted_pathname("");

  ArgParser options(argc, argv);
  options.get_args(sd_pathname, hamms_sphere_pathname, atom_scale,
                   atom_centers_only, measure_ids, tversky_alpha,
                   sorted_pathname);

  MeasuresList measures;
  get_measures(measure_ids, tversky_alpha, measures);
  if (measures.empty()) {
    options.show_usage("No valid measures specified");
  }
  SDFMolAligner aligner(sd_pathname, hamms_sphere_pathname, atom_scale,
                        atom_centers_only, measures, sorted_pathname);
  aligner.run();
  return 0;
}
