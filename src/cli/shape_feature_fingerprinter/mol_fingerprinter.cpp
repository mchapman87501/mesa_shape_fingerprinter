//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mol_fingerprinter.h"

#include <cmath>
#include <cstdlib>

#include "shape_feature_fingerprinter/feature_finder_factory.h"

using namespace std;

namespace mesaac {
const static float C_FlipMatrix[4][3] = {{1.0, 1.0, 1.0}, // Unflipped
                                         {1.0, -1.0, -1.0},
                                         {-1.0, 1.0, -1.0},
                                         {-1.0, -1.0, 1.0}};
const static unsigned int C_FlipMatrixSize =
    sizeof(C_FlipMatrix) / sizeof(C_FlipMatrix[0]);

MolFingerprinter::MolFingerprinter(PointList &hammsEllipsoidCoords,
                                   PointList &hammsSphereCoords,
                                   float epsilonSqr, unsigned int numFolds)
    : m_axisAligner(hammsSphereCoords, epsilonSqr, true),
      m_volBox(hammsEllipsoidCoords, epsilonSqr), m_numFolds(numFolds), m_ff(0),
      m_iFlip(0) {}

MolFingerprinter::~MolFingerprinter() {
  if (m_ff) {
    delete m_ff;
    m_ff = 0;
  }
}

void MolFingerprinter::updateFF() {
  if (m_ff) {
    delete m_ff;
  }

  ostringstream outs;
  mesaac::mol::SDWriter writer(outs);
  writer.write(m_mol);

  m_ff = create_feature_finder(outs.str());
}

MolFingerprinter::MolFingerprinter(const MolFingerprinter &src)
    : m_axisAligner(src.m_axisAligner), m_volBox(src.m_volBox),
      m_numFolds(src.m_numFolds), m_mol(src.m_mol), m_iFlip(src.m_iFlip),
      m_heavies(src.m_heavies), m_allAtoms(src.m_allAtoms) {
  // Bad form?  calling mfunc from a copy constructor.
  // Can't directly copy FeatureFinder, hence this.
  updateFF();
}

void MolFingerprinter::setMolecule(mol::Mol &mol) {
  m_mol = mol;
  m_iFlip = 0;
  m_axisAligner.align_to_axes(m_mol);

  // Need heavy atoms for shape fingerprinting.
  m_heavies.clear();
  m_axisAligner.get_atom_points(m_mol.atoms(), m_heavies, false);

  // Need all atoms for feature fingerprinting.
  m_allAtoms.clear();
  m_axisAligner.get_atom_points(m_mol.atoms(), m_allAtoms, true);

  updateFF();
}

bool MolFingerprinter::getNextFP(BitVector &fp) {
  bool result(false);

  if (m_iFlip != C_FlipMatrixSize) {
    computeCurrFlipFingerprint(fp);
    m_iFlip++;
    result = true;
  }
  return result;
}

namespace {
inline void getFlippedPoints(const PointList &points, const float *flip,
                             PointList &flippedPoints) {
  flippedPoints = points;

  PointList::iterator iEnd(flippedPoints.end());
  PointList::iterator i;
  for (i = flippedPoints.begin(); i != iEnd; ++i) {
    shape::Point &p(*i);
    p[0] *= flip[0];
    p[1] *= flip[1];
    p[2] *= flip[2];
  }
}

} // namespace

void MolFingerprinter::computeCurrFlipFingerprint(BitVector &result) {
  const float *flip = C_FlipMatrix[m_iFlip];
  PointList flippedPoints;

  // How many points are in the volbox?  The fingerprint will need
  // to have that many bits for the shape, and for each feature type.
  unsigned int num_feature_types = m_ff->num_feature_types();
  unsigned int num_cloud_points = m_volBox.size();
  unsigned int fp_size = num_cloud_points * (num_feature_types + 1);
  // Accommodate folding.
  fp_size /= (1 << m_numFolds);
  unsigned int stride = num_cloud_points / (1 << m_numFolds);
  result.resize(fp_size);
  result.reset();

  // Shape fingerprint component:
  getFlippedPoints(m_heavies, flip, flippedPoints);
  m_volBox.set_folded_bits_for_spheres(flippedPoints, result, m_numFolds, 0);

  // Feature fingerprint components:
  if (m_ff) {
    getFlippedPoints(m_allAtoms, flip, flippedPoints);
    unsigned int offset = stride;
    unsigned int i;
    for (i = 0; i != num_feature_types; ++i) {
      IFeatureFinder::IntVector iAtoms;
      m_ff->get_feature_atom_indices(i, iAtoms);
      PointList featureAtoms;
      IFeatureFinder::IntVector::iterator j;
      for (j = iAtoms.begin(); j != iAtoms.end(); ++j) {
        // Add the flipped coords for the indexed feature atom.
        featureAtoms.push_back(flippedPoints.at(*j));
      }
      m_volBox.set_folded_bits_for_spheres(featureAtoms, result, m_numFolds,
                                           offset);
      offset += stride;
    }
  }
}
} // namespace mesaac
