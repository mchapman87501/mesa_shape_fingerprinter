// Generate sparse (dis)similarity matrix bands from a set of shape
// fingerprints.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <sstream>
#include <string>
#include <vector>

#include "Globals.h"
#include "build_info.h"
#include "mesaac_measures/measurer_factory.h"

#include "mesaac_common/b64.h"
#include "mesaac_common/gzip.h"
#include "mesaac_common/mesaac_common.h"

#include "fp_decoder.h"

using namespace std;

string Version = "1.0";
string CreationDate = "November, 2010";

static void show_blurb(char *exename) {
  cerr << "Running " << basename(exename) << endl
       << "Source code Copyright (c) 2009" << endl
       << "Mesa Analytics & Computing, Inc." << endl
       << "Revision: " << BUILD_REVISION << " Built: " << BUILD_DATE << endl
       << "Expiration Date: " << mesaac::expirationDateStr();
}

void read_fpblocks_from_stream(const string &pathname, istream &ins,
                               ShapeFPBlocks &fingerprints) {
  ArrayBitVectors block;
  const unsigned int FPsPerBlock = 4;
  bool first = true;
  unsigned int vector_size = 0;
  string fpstr;
  unsigned int line_num = 0;

  fingerprints.clear();
  while (ins >> fpstr) {
    line_num++;
    BitVector fp;
    if (!decode_fp(fpstr, fp)) {
      cerr << "Error at line " << line_num << " of " << pathname << ":" << endl
           << "  Invalid fingerprint string '" << fpstr << "'." << endl;
      exit(1);
    } else {
      if (first) {
        vector_size = fp.size();
        first = false;
      } else if (fp.size() != vector_size) {
        // TODO:  Use exceptions, or an error return
        cerr << "Error at line " << line_num << " of " << pathname << ":"
             << endl
             << "  Expected fingerprint of size " << vector_size
             << ", got fingerprint of size " << fp.size() << endl;
        exit(1);
      }
      block.push_back(BitVector(fp));
      if (block.size() == FPsPerBlock) {
        fingerprints.push_back(block);
        block.clear();
      }
    }
  }
  // Discard any leftover sub-block.
  // cerr << "Number of fingerprints is " << fingerprints.size() << endl
  //      << fingerprints.size() << " " << FPsPerBlock << " " << vector_size
  //      << endl;

  if (block.size() != 0) {
    cerr << "A Shape Fingerprint file must contain blocks of " << FPsPerBlock
         << " fingerprints.  " << endl
         << pathname << " has only " << block.size()
         << ((block.size() == 1) ? "fingerprint " : "fingerprints ")
         << "in its last block." << endl
         << "This may not be a shape fingerprint file." << endl;
    exit(1);
  }
}

void read_fingerprint_blocks(const string pathname,
                             ShapeFPBlocks &fingerprints) {
  // Input from either stdin or file
  if (pathname == "-") {
    read_fpblocks_from_stream("standard input", cin, fingerprints);
  } else {
    ifstream inf(pathname.c_str());
    if (!inf) {
      // TODO:  Use exceptions, or an error return
      cerr << "Cannot open fingerprint file " << pathname << "." << endl;
      exit(1);
    }
    read_fpblocks_from_stream(pathname, inf, fingerprints);
    inf.close();
  }
}

class ArgParser {
public:
  ArgParser(int argc, char **argv) : m_argc(argc), m_argv(argv) {}

  virtual ~ArgParser() {}

  void parse_args(string &sfp_pathname, string &measure_name, bool &compute_sim,
                  float &tversky_alpha, float &sparse_threshold,
                  unsigned int &start_row, unsigned int &end_row,
                  string &format) {
    measure_name = "T";
    compute_sim = false;
    tversky_alpha = 0.0;
    sparse_threshold = 1.0;
    start_row = 0;
    end_row = 0;
    format = "S";

    bool alpha_specified = false;
    bool thresh_specified = false;

    int i = 1;
    while (i < m_argc) {
      const string opt(m_argv[i]);
      if (opt == "-m" || opt == "--measure") {
        i += 1;
        parse_val("MEASURE", i, measure_name);
        // Let get_shape_measurer validate the measure type.
      } else if (opt == "-a" || opt == "--alpha") {
        i += 1;
        parse_val("ALPHA", i, tversky_alpha, 0.0, 2.0);
        alpha_specified = true;
      } else if (opt == "-d" || opt == "--dist") {
        i += 1;
        compute_sim = false;
      } else if (opt == "-s" || opt == "--sim") {
        i += 1;
        compute_sim = true;
      } else if (opt == "-r" || opt == "--records") {
        i += 1;
        parse_val("START", i, start_row, 0);
        parse_val("END", i, end_row, start_row + 1);
      } else if (opt == "-f" || opt == "--format") {
        i += 1;
        parse_val("FORMAT", i, format);
        if (format != "S" && format != "M") {
          m_msg << "Invalid FORMAT " << format;
          show_usage_and_exit();
        }
      } else if (opt == "-t" || opt == "--threshold") {
        i += 1;
        parse_val("THRESHOLD", i, sparse_threshold, 0.0, 1.0);
        thresh_specified = true;
      } else if (opt == "-h" || opt == "--help") {
        show_help_msg();
      } else if (opt != "-" && opt.substr(1, 1) == "-") {
        // '-' is a positional argument denoting standard input.
        // '-' followed by any other character is an option.
        m_msg << "Unsupported option " << opt;
        show_usage_and_exit();
      } else {
        break;
      }
    }

    // Only required arguments remain.
    parse_val("FILENAME", i, sfp_pathname);

    if (i < m_argc) {
      m_msg << "Too many arguments";
      show_usage_and_exit();
    }

    // Warnings...
    m_msg.str("");
    if (alpha_specified && (measure_name != "V")) {
      m_msg << "A measure other than Tversky was used.  Ignoring ALPHA."
            << endl;
    }

    // Set default threshold.
    if (format == "S" && !thresh_specified) {
      // Equivalent to full matrix.
      string similarity("similarity");
      if (compute_sim) {
        sparse_threshold = 0.0;
      } else {
        similarity = "distance";
        sparse_threshold = 1.0;
      }

      m_msg << "No sparse THRESHOLD was given.  The default " << similarity
            << " threshold, " << sparse_threshold << ", will be used." << endl;
    } else if (format == "M" && thresh_specified) {
      m_msg << "Full matrix FORMAT ('-f M') was specified.  "
            << "Sparse threshold will be ignored." << endl;
    }
    if (m_msg.str().size()) {
      show_usage();
    }
  }

  void warn(const string msg) {
    m_msg.str(msg);
    show_usage();
  }

  void error(const string msg) {
    m_msg.str(msg);
    show_usage_and_exit();
  }

protected:
  int m_argc;
  char **m_argv;
  ostringstream m_msg; // Error message stream.

  void show_usage() {
    const string msg(m_msg.str());
    if (msg.size()) {
      cerr << msg << endl;
    }
    m_msg.str(""); // In case there are more warnings.

    string exename = basename(m_argv[0]);
    cerr << "use \"" << exename << " --help\" for more information." << endl
         << endl;
  }

  void show_usage_and_exit() {
    show_usage();
    exit(1);
  }

  void show_help_msg() {
    string exename = basename(m_argv[0]);
    cerr
        << "Usage: " << exename
        << " [-m | --measure MEASURE] [-a | --alpha ALPHA]\n\
    [-d | --dist | -s | --sim] [-r | --records START END] [-f | --format FORMAT]\n\
    [-t | --threshold THRESHOLD] FILENAME\n\
Print pairwise similarities or distances between shape fingerprints.\n\
\n\
-m | --measure MEASURE\n\
    The MEASURE with which to compute the matrix.  Valid values are 'T'\n\
    (Tanimoto, the default), 'V' (Tversky), 'E' (Euclidean), 'H' (Hamann),\n\
    'C' (Cosine) and 'B' (BUB).\n\
-a | --alpha ALPHA\n\
    When the Tversky measure ('-m V') is used, a Tversky ALPHA value in the\n\
    range 0..2, inclusive, may be specified.  If ALPHA is not specified, a\n\
    default value of 0 is used.\n\
    NOTES:  The Tversky beta value is automatically set to 2 - ALPHA.\n\
            Setting ALPHA = beta = 1 is equivalent to using the Tanimoto\n\
            measure ('-m T').\n\
-d | --dist | -s | --sim\n\
    Specify whether to compute distances (the default) or similarities.\n\
-r | --records START END\n\
    Output rows START..(END - 1), inclusive, of the matrix.  By default all\n\
    rows, 0..(# fingerprints - 1), are output.\n\
-f | --format FORMAT\n\
    Write results in sparse matrix format ('S', the default) or full matrix\n\
    format ('M')\n\
-t | --threshold THRESHOLD\n\
    When writing results in sparse matrix format ('-f S') a sparse threshold\n\
    may be specified.  Valid values are in the range 0..1, inclusive.\n\
    For similarity matrices only values greater than or equal to THRESHOLD\n\
    are output.  For distance matrices only values less than or equal to\n\
    THRESHOLD are output.\n\
    If no THRESHOLD is specified for sparse matrix output, a default value\n\
    of 0 (for similarity matrices) or 1 (for distance matrices) will be used.\n\
-h | --help\n\
    Show this help message and exit.\n\
\n\
FILENAME\n\
    Path to the shape fingerprint file.  To read from stdin use '-'.\n\
\n\
Examples:"
        << endl
        << "    $ " << exename << " -t 0.4 shape_fingerprints.txt" << endl
        << "Write out a complete Tanimoto sparse distance matrix.  Use shape fingerprints\n\
from shape_fingerprints.txt.  The matrix will include all pairwise distances\n\
less than or equal to 0.4."
        << endl
        << endl;

    const string msg(m_msg.str());
    if (msg.size()) {
      cerr << endl << msg << endl;
    }
    exit(0);
  }

  void parse_val(string option_name, int &i, string &sval) {
    if (i >= m_argc) {
      m_msg << "No value specified for " << option_name;
      show_usage_and_exit();
    } else {
      sval = m_argv[i];
    }
    i++;
  }

  void parse_val(string option_name, int &i, unsigned int &ival) {
    string sval;
    parse_val(option_name, i, sval);
    istringstream ins(sval);
    if (!(ins >> ival)) {
      m_msg << "Value provided for " << option_name << " ('" << sval
            << "') is not a valid integer.";
      show_usage_and_exit();
    }
  }

  void parse_val(string option_name, int &i, float &fval) {
    string sval;
    parse_val(option_name, i, sval);
    istringstream ins(sval);
    if (!(ins >> fval)) {
      m_msg << "Value provided for " << option_name << " ('" << sval
            << "') is not a valid number.";
      show_usage_and_exit();
    }
  }

  void parse_val(string option_name, int &i, unsigned int &ival,
                 unsigned int min_val) {
    parse_val(option_name, i, ival);
    if (ival < min_val) {
      m_msg << option_name << " (" << ival << ") must be >= " << min_val;
      show_usage_and_exit();
    }
  }

  void parse_val(string option_name, int &i, unsigned int &ival,
                 unsigned int min_val, unsigned int max_val) {
    parse_val(option_name, i, ival, min_val);
    if (ival > max_val) {
      m_msg << option_name << " (" << ival << ") must be <= " << max_val;
      show_usage_and_exit();
    }
  }

  void parse_val(string option_name, int &i, float &fval, float min_val,
                 float max_val) {
    parse_val(option_name, i, fval);
    if (min_val > fval || fval > max_val) {
      m_msg << option_name << " (" << fval << ") must be in the range "
            << min_val << ".." << max_val << ", inclusive.";
      show_usage_and_exit();
    }
  }
};

static void write_sparse_matrix(IMeasurer *measurer,
                                unsigned int number_shapefingerprints,
                                unsigned int start_row, unsigned int end_row,
                                bool compute_sim, float sparse_threshold) {
  if (compute_sim) {
    for (unsigned int i = start_row; i < end_row; ++i) {
      for (unsigned int j = 0; j < number_shapefingerprints; ++j) {
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
    for (unsigned int i = start_row; i < end_row; ++i) {
      for (unsigned int j = 0; j < number_shapefingerprints; ++j) {
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
}

static void write_full_matrix(IMeasurer *measurer,
                              unsigned int number_shapefingerprints,
                              unsigned int start_row, unsigned int end_row) {
  for (unsigned int i = start_row; i < end_row; ++i) {
    string sep("");
    for (unsigned int j = 0; j < number_shapefingerprints; ++j) {
      cout << sep << measurer->value(i, j);
      sep = " ";
    }
    cout << endl;
  }
}

int main(int argc, char **argv) {
  mesaac::MesaacFeatures f;
  f.set(mesaac::MFE_GroupingModule);
  mesaac::initCommon(f); // Validate license.

  string fp_pathname = "-";
  string dist_type = "T";
  bool compute_sim = false;
  float tversky_alpha = 0.0;
  float sparse_threshold = 1.0;
  unsigned int start_row, end_row;
  string format = "S";

  ArgParser arg_parser(argc, argv);
  arg_parser.parse_args(fp_pathname, dist_type, compute_sim, tversky_alpha,
                        sparse_threshold, start_row, end_row, format);
  show_blurb(argv[0]);

  // start_row and end_row should both be zero now; or they should
  // be values explicitly specified by the user.

  ShapeFPBlocks fingerprints;
  read_fingerprint_blocks(fp_pathname, fingerprints);

  IMeasurer *measurer =
      get_shape_measurer(compute_sim, dist_type, fingerprints, tversky_alpha);
  if (0 == measurer) {
    ostringstream msg;
    msg << "Could not create measurer " << dist_type;
    arg_parser.error(msg.str());
  }

  // No end row specified?  Output everything.
  unsigned int number_shapefingerprints = fingerprints.size();
  if (end_row == 0) {
    end_row = number_shapefingerprints;
  }
  if (start_row > end_row || end_row > number_shapefingerprints) {
    ostringstream msg;
    msg << "Cannot generate sparse rows " << start_row << ".." << end_row
        << " - 1.  Fingerprint file contains only " << number_shapefingerprints
        << " shape fingerprints.";
    arg_parser.error(msg.str());
  }

  if ("S" == format) {
    write_sparse_matrix(measurer, number_shapefingerprints, start_row, end_row,
                        compute_sim, sparse_threshold);
  } else if ("M" == format) {
    write_full_matrix(measurer, number_shapefingerprints, start_row, end_row);
  } else {
    ostringstream msg;
    msg << "Invalid FORMAT " << format << ".  Please specify 'M' or 'S'.";
    arg_parser.error(msg.str());
  }

  return 0;
}
