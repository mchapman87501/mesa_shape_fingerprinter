// Shape Fingerprinter using quasi-Monte Carlo
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.
//
// This program takes as input an sd file of conformers, a file of Hammersly
// points arrayed in a scalene ellipsoid of a size into which small molecules
// will fit, in an orientation whereby the small molecule is aligned to the
// principle component.

// Step
// 1.  Find Hammersly points in atom defined volume of conformer and add to
//     atom center points.  Call this point data matrix X.
//
// 2.  Calculate eigenvectors and eigenvalues of X, and return X' rotated
//     to the eigenvector axes.
//
// 3.  Rotate X' minus atom points about eigenvector axes to obtain 8
//     fingerprints of length |rows of X'| minus atom points -- a bit
//     per point, such that a bit is 1 if the point is in the volume of the
//     conformer, 0 if the point lies outside any of the atoms (volume) of
//     the conformer.
//
// 4.  Output 4 fingerprints per conformer (optionally, output eigenvectors
//     eigenvalues, and rotation matrix per conformer).
//

#include <iostream>
#include <libgen.h>
#include <sstream>
#include <string>

#include "sdf_shape_fingerprinter.hpp"
#include "shared_types.hpp"

using namespace std;

namespace {
using namespace mesaac::shape_fingerprinter;

class ArgParser {
public:
  ArgParser(int argc, char **argv) : m_argc(argc), m_argv(argv) {}
  virtual ~ArgParser() {}

  void get_args(string &sd_pathname, string &he_pathname, string &hs_pathname,
                float &atom_scale, bool &include_ids,
                SDFShapeFingerprinter::FormatEnum &format,
                unsigned int &num_folds, int &start_index, int &end_index) {
    include_ids = false;
    bool custom_he_path = false;
    format = SDFShapeFingerprinter::FMT_ASCII;
    num_folds = 0;
    start_index = 0; // Start at first record
    end_index = -1;  // Never stop

    int i = 1;
    while (i < m_argc) {
      string curr_arg(m_argv[i]);
      if (curr_arg.substr(0, 1) != "-") {
        break;
      }
      if ((curr_arg == "-i") || (curr_arg == "--id")) {
        include_ids = true;
      } else if ((curr_arg == "-f") || (curr_arg == "--format")) {
        string format_str;
        get_value_for("FORMAT", i, format_str);

        if ("A" == format_str) {
          format = SDFShapeFingerprinter::FMT_ASCII;
        } else if ("C" == format_str) {
          format = SDFShapeFingerprinter::FMT_COMPRESSED_ASCII;
        } else if ("B" == format_str) {
          format = SDFShapeFingerprinter::FMT_BINARY;
        } else {
          ostringstream msg;
          msg << "Unsupported format '" << format_str << "'.";
          show_usage(msg);
        }
      } else if ((curr_arg == "-n") || (curr_arg == "--num_folds")) {
        get_value_for("FOLDS", i, num_folds);
      } else if ((curr_arg == "-e") || (curr_arg == "--ellipsoid")) {
        get_value_for("ELLIPSOID_FILE", i, he_pathname);
        custom_he_path = true;
      } else if ((curr_arg == "-r") || (curr_arg == "--records")) {
        get_value_for("START", i, start_index);
        get_value_for("END", i, end_index);
      } else if ((curr_arg == "-h") || (curr_arg == "--help")) {
        show_usage("", 0);
      } else {
        ostringstream msg;
        msg << "Unsupported option '" << curr_arg << "'";
        show_usage(msg);
      }
      i++;
    }

    if ((m_argc - i) != 3) {
      show_usage("Wrong number of arguments");
    }
    // By default, the ellipsoid pathname should be the same as the spheroid
    // pathname.
    sd_pathname = m_argv[i++];
    hs_pathname = m_argv[i++];
    if (!custom_he_path) {
      he_pathname = hs_pathname;
    }

    char *atom_scale_arg(m_argv[i++]);
    istringstream ins(atom_scale_arg);
    ins >> atom_scale;
    if (!ins) {
      ostringstream err;
      err << "Value for atom_scale ('" << atom_scale_arg
          << "') is not a float.";
      show_usage(err);
    }
    if ((atom_scale < 1.0) || (atom_scale > 2.0)) {
      ostringstream err;
      err << "atom_scale must be in the range [1.0 .. 2.0]";
      show_usage(err);
    }
  }

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

  void get_value_for(string option_name, int &i, unsigned int &uival) {
    string sval;
    get_value_for(option_name, i, sval);
    istringstream ins(sval);
    if (!(ins >> uival)) {
      ostringstream msg;
      msg << "Invalid integer value '" << sval << "' for " << option_name;
      show_usage(msg);
    }
  }

  void get_value_for(string option_name, int &i, int &ival) {
    string sval;
    get_value_for(option_name, i, sval);
    istringstream ins(sval);
    if (!(ins >> ival)) {
      ostringstream msg;
      msg << "Invalid integer value '" << sval << "' for " << option_name;
      show_usage(msg);
    }
  }

  void show_usage(string err_msg = "", int status = 1) {
    cerr << "Usage: " << basename(m_argv[0])
         << " [options] sd_file hamms_sphere_file atom_scale\n\
Generate shape fingerprints for 3D conformers.\n\
\n\
sd_file              - a file of conformers in SD format, with 3D coordinates\n\
hamms_sphere_file    - a file containing 3D Hammersley sphere points, one point \n\
                       per line with space-separated coordinates, for principal \n\
                       axes generation via SVD and fingerprint generation\n\
atom_scale           - the amount, in the range [1.0 .. 2.0], by which to \n\
                       increase atom radii for alignment\n\
\n\
Options:\n\
-i | --id            - if specified, include the name of each SD conformer \n\
                       after each fingerprint, separated by a space\n\
-f fmt | --format fmt\n\
                     - write fingerprints in the specified format: \n\
                       A - ascii (default)\n\
                       C - compressed ascii\n\
                       B - binary\n\
-n folds | --num_folds folds\n\
                     - fold fingerprints the specified number of times,\n\
                       to save space on output.  The default is zero\n\
                       (unfolded).\n\
-e | --ellipsoid ELLIPSOID_FILE\n\
                     - use points from ELLIPSOID_FILE, a file containing 3D\n\
                       Hammersley ellipsoid points, one point per line with \n\
                       space-separated coords, for fingerprint generation\n\
-r | --records START END\n\
                     - process records START..(END - 1), inclusive, of the\n\
                       sd_file.  By default all records, \n\
                       0..(# fingerprints - 1), are processed.\n\
-h | --help          - print this help message and exit\n"
         << endl;

    if (err_msg.size()) {
      cerr << endl << err_msg << endl;
    }
    exit(status);
  }
  void show_usage(const char *err_msg) { show_usage(string(err_msg)); }
  void show_usage(ostringstream &err_msg) { show_usage(err_msg.str()); }
};

string Version = "1.0";
string CreationDate = "April 30, 2009";

} // namespace

int main(int argc, char **argv) {
  string sd_pathname;
  string hamms_ellipsoid_path;
  string hamms_spheroid_path;
  float atom_scale = 1.0;
  bool include_ids = false;
  mesaac::shape_fingerprinter::SDFShapeFingerprinter::FormatEnum format;
  unsigned int num_folds = 0;
  int start_index = 0, end_index = -1;

  ArgParser options(argc, argv);
  options.get_args(sd_pathname, hamms_ellipsoid_path, hamms_spheroid_path,
                   atom_scale, include_ids, format, num_folds, start_index,
                   end_index);

  SDFShapeFingerprinter sfper(sd_pathname, hamms_ellipsoid_path,
                              hamms_spheroid_path, atom_scale, include_ids,
                              format, num_folds);
  sfper.run(start_index, end_index);
  return 0;
}
