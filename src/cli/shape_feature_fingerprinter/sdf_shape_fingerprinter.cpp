//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "sdf_shape_fingerprinter.h"

#include "mesaac_common/b64.h"
#include "mesaac_common/gzip.h"
#include <fstream>
#include <iostream>
#include <sstream>

#include "mol_fingerprinter.h"

using namespace std;

namespace mesaac {
static void openInput(ifstream &inf, string &pathname, string description) {
  inf.open(pathname);
  if (!inf) {
    cerr << "Cannot open " << description << " '" << pathname
         << "' for reading." << endl;
    exit(1);
  }
}

SDFShapeFingerprinter::SDFShapeFingerprinter(
    string sdPathname, string hammsEllipsoidPathname,
    string hammsSpherePathname, float radiiEpsilon, bool includeIDs,
    SDFShapeFingerprinter::FormatEnum format, unsigned int numFolds)
    : m_sdPathname(sdPathname),
      m_hammsEllipsoidPathname(hammsEllipsoidPathname),
      m_hammsSpherePathname(hammsSpherePathname),
      m_epsilonSqr(radiiEpsilon * radiiEpsilon), m_includeIDs(includeIDs),
      m_format(format), m_numFolds(numFolds) {}

SDFShapeFingerprinter::~SDFShapeFingerprinter() {}

static void readPoints(string &pathname, string description,
                       PointList &points) {
  ifstream inf;
  openInput(inf, pathname, description);

  float coord;
  while (inf >> coord) {
    FloatVector p(3, 0.0);
    p[0] = coord;
    inf >> p[1] >> p[2];
    points.push_back(p);
  }
  inf.close();
}

void SDFShapeFingerprinter::run(int startIndex, int endIndex) {
  PointList ellipsoid, sphere;
  readPoints(m_hammsEllipsoidPathname, "hamms_ellipsoid_filename", ellipsoid);
  readPoints(m_hammsSpherePathname, "hamms_sphere_filename", sphere);
  processMolecules(ellipsoid, sphere, startIndex, endIndex);
}

static inline string compressed_fp(BitVector &fp) {
  typedef BitVector::block_type Block;
  Block buffer[fp.num_blocks()];

  boost::to_block_range(fp, buffer);

  ostringstream str(ios_base::binary | ios_base::out);
  for (size_t i = 0; i != fp.num_blocks(); ++i) {
    str.write((const char *)&(buffer[i]), sizeof(Block));
  }

  B64 codec;
  return codec.encode(gzip::compress(str.str()));
}

static inline string cbinascii_fp(BitVector &fp) {
  B64 codec;
  string s;
  boost::to_string(fp, s);
  return codec.encode(gzip::compress(s));
}

void SDFShapeFingerprinter::processMolecules(PointList &ellipsoid,
                                             PointList &sphere, int startIndex,
                                             int endIndex) {
  if (startIndex < 0) {
    cerr << "Invalid start index " << startIndex << " -- must be >= 0" << endl;
    exit(1);
  }

  ifstream inf(m_sdPathname);
  if (!inf) {
    // TODO: throw exception
    cerr << "Cannot open sd file '" << m_sdPathname << "'" << endl;
    exit(1);
  }
  mol::SDReader reader(inf, m_sdPathname);
  MolFingerprinter mfp(ellipsoid, sphere, m_epsilonSqr, m_numFolds);

  int i = 0;
  while (i < startIndex && reader.skip()) {
    i += 1;
  }

  // If endIndex < 0, just process everything.
  mol::Mol mol;
  switch (m_format) {
  case FMT_COMPRESSED_ASCII:
    while (((endIndex < 0) || (i < endIndex)) && reader.read(mol)) {
      mfp.setMolecule(mol);
      BitVector fp;
      while (mfp.getNextFP(fp)) {
        cout << "C" << cbinascii_fp(fp);
        if (m_includeIDs) {
          cout << " " << mol.name();
        }
        cout << endl;
      }
      ++i;
    }
    break;

  case FMT_BINARY:
    while (((endIndex < 0) || (i < endIndex)) && reader.read(mol)) {
      mfp.setMolecule(mol);
      BitVector fp;
      while (mfp.getNextFP(fp)) {
        cout << "B" << compressed_fp(fp);
        if (m_includeIDs) {
          cout << " " << mol.name();
        }
        cout << endl;
      }
      ++i;
    }
    break;

  case FMT_ASCII:
  default:
    while (((endIndex < 0) || (i < endIndex)) && reader.read(mol)) {
      mfp.setMolecule(mol);
      BitVector fp;
      while (mfp.getNextFP(fp)) {
        cout << fp;
        if (m_includeIDs) {
          cout << " " << mol.name();
        }
        cout << endl;
      }
      ++i;
    }
    break;
  }
  inf.close();
}
} // namespace mesaac
