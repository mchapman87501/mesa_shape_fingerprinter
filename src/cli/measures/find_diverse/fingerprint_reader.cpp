//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "fingerprint_reader.h"

#include <sstream>

using namespace std;

namespace mesaac {
FingerprintReader::FingerprintReader(string &pathname) {
  m_inf.open(pathname);
  // TODO: Use exceptions or error codes.
  if (!m_inf) {
    cerr << "Could not open fingerprint file '" << pathname << "' for reading."
         << endl;
    exit(1);
  }
  m_pathname = pathname;
  m_fpnum = 0;
  m_fplen = 0;
}

FingerprintReader::~FingerprintReader() { m_inf.close(); }

static inline bool read_one(istream &inf, unsigned int &index,
                            unsigned int &fplen, BitVector &fp) {
  bool result = false;
  string fpstr;
  if (inf && (inf >> fpstr)) {
    index++;

    if (fpstr.find_first_not_of("01") != string::npos) {
      // TODO Use exceptions or an error return
      cerr << "Error at line " << index << ":" << endl
           << "  Fingerprints must contain only '0' and '1'"
           << " characters." << endl;
      exit(1);
    }

    if (1 == index) {
      fplen = fpstr.size();
    } else if (fpstr.size() != fplen) {
      cerr << "Error at line " << index << ":" << endl
           << "Fingerprint should be of length " << fplen << ", but has length "
           << fpstr.size();
      exit(1);
    }
    // What a waste...  But dynamic_bitset fails clumsily
    // if you feed it bad input directly.
    fp = BitVector(fpstr);
    result = true;
  }
  return result;
}

bool FingerprintReader::next(BitVector &fp) {
  return read_one(m_inf, m_fpnum, m_fplen, fp);
}

void FingerprintReader::read_all(ArrayBitVectors &all_fingerprints) {
  all_fingerprints.clear();
  BitVector nextfp;
  while (read_one(m_inf, m_fpnum, m_fplen, nextfp)) {
    all_fingerprints.push_back(nextfp);
  }
}
} // namespace mesaac
