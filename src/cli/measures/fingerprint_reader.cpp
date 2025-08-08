//
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "fingerprint_reader.hpp"

#include <fstream>
#include <iostream>

using namespace std;

namespace mesaac::cli::measures {
namespace {
void read_fingerprints_from_stream(const string &pathname, istream &ins,
                                   shape_defs::ArrayBitVectors &fingerprints) {
  fingerprints.clear();

  bool first = true;
  unsigned int vector_size = 0;
  string fp;
  while (ins >> fp) {
    if (first) {
      vector_size = fp.size();
      first = false;
    } else if (fp.size() != vector_size) {
      // TODO:  Use exceptions, or an error return
      cerr << "Fingerprints of unequal size from " << pathname << endl;
      exit(1);
    }
    if (fp.find_first_not_of("01") != string::npos) {
      // TODO:  Use exceptions, or an error return
      cerr << "Error at line " << fingerprints.size() + 1 << " of " << pathname
           << ":" << endl
           << "  Fingerprints must contain only '0' and '1' characters" << endl;
      exit(1);
    }
    fingerprints.push_back(shape_defs::BitVector(fp));
  }
}

} // namespace

void read_fingerprints(const string &pathname,
                       shape_defs::ArrayBitVectors &fingerprints) {
  // Input from either stdin or file
  if (pathname == "-") {
    read_fingerprints_from_stream("standard input", cin, fingerprints);
  } else {
    ifstream inf(pathname);
    if (!inf) {
      // TODO:  Use exceptions, or an error return
      cerr << "Cannot open fingerprint file " << pathname << "." << endl;
      exit(1);
    }
    read_fingerprints_from_stream(pathname, inf, fingerprints);
    inf.close();
  }
}
} // namespace mesaac::cli::measures