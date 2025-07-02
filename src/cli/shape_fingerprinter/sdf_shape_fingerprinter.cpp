//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "sdf_shape_fingerprinter.hpp"

#include "mesaac_common/b64.hpp"
#include "mesaac_common/gzip.hpp"
#include "mesaac_mol/io/sdreader.hpp"

#include <fstream>
#include <iostream>
#include <sstream>

#include "mol_fingerprinter.hpp"

using namespace std;

namespace mesaac::shape_fingerprinter {
namespace {
void open_input(ifstream &inf, string &pathname, string description) {
  inf.open(pathname.c_str());
  if (!inf) {
    cerr << "Cannot open " << description << " '" << pathname
         << "' for reading." << endl;
    exit(1);
  }
}

void read_points(string &pathname, string description, PointList &points) {
  ifstream inf;
  open_input(inf, pathname, description);

  float coord;
  while (inf >> coord) {
    FloatVector p(3, 0.0);
    p[0] = coord;
    inf >> p[1] >> p[2];
    points.push_back(p);
  }
  inf.close();
}

inline string compressed_fp(shape_defs::BitVector &fp) {
  using Block = shape_defs::BitVector::block_type;
  Block buffer[fp.num_blocks()];

  boost::to_block_range(fp, buffer);

  ostringstream str(ios_base::binary | ios_base::out);
  for (size_t i = 0; i != fp.num_blocks(); ++i) {
    str.write((const char *)&(buffer[i]), sizeof(Block));
  }

  common::B64 codec;
  return codec.encode(common::gzip::compress(str.str()));
}

inline string cbinascii_fp(shape_defs::BitVector &fp) {
  common::B64 codec;
  string s;
  boost::to_string(fp, s);
  return codec.encode(common::gzip::compress(s));
}

} // namespace
SDFShapeFingerprinter::SDFShapeFingerprinter(
    string sd_pathname, string hamms_ellipsoid_pathname,
    string hamms_sphere_pathname, float radii_epsilon, bool include_ids,
    FormatEnum format, unsigned int num_folds)
    : m_sd_pathname(sd_pathname),
      m_hamms_ellipsoid_pathname(hamms_ellipsoid_pathname),
      m_hamms_sphere_pathname(hamms_sphere_pathname),
      m_epsilon_sqr(radii_epsilon * radii_epsilon), m_include_ids(include_ids),
      m_format(format), m_num_folds(num_folds) {}

void SDFShapeFingerprinter::run(int start_index, int end_index) {
  PointList ellipsoid, sphere;
  read_points(m_hamms_ellipsoid_pathname, "hamms_ellipsoid_filename",
              ellipsoid);
  read_points(m_hamms_sphere_pathname, "hamms_sphere_filename", sphere);
  process_molecules(ellipsoid, sphere, start_index, end_index);
}

void SDFShapeFingerprinter::process_molecules(PointList &ellipsoid,
                                              PointList &sphere,
                                              int start_index, int end_index) {
  if (start_index < 0) {
    cerr << "Invalid start index " << start_index << " -- must be >= 0" << endl;
    exit(1);
  }
  ifstream inf(m_sd_pathname.c_str());
  if (!inf) {
    // TODO: throw exception
    cerr << "Cannot open sd file '" << m_sd_pathname << "'" << endl;
    exit(1);
  }
  mol::SDReader reader(inf, m_sd_pathname);
  MolFingerprinter mfp(ellipsoid, sphere, m_epsilon_sqr, m_num_folds);

  int i = 0;
  while (i < start_index && reader.skip()) {
    i += 1;
  }

  // If end_index < 0, just process everything.
  mol::Mol mol;
  while (((end_index < 0) || (i < end_index)) && reader.read(mol)) {
    mfp.set_molecule(mol);
    shape_defs::BitVector fp;
    while (mfp.get_next_fp(fp)) {
      switch (m_format) {
      case FMT_COMPRESSED_ASCII:
        cout << "C" << cbinascii_fp(fp);
        break;

      case FMT_BINARY:
        cout << "B" << compressed_fp(fp);
        break;

      case FMT_ASCII:
      default:
        cout << fp;
        break;
      }
      if (m_include_ids) {
        cout << " " << mol.name();
      }
      cout << endl;
    }
    ++i;
  }
  inf.close();
}
} // namespace mesaac::shape_fingerprinter
