//
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mol_fingerprinter.h"

#include <cstdlib>
#include <cmath>

using namespace std;

namespace mesaac {
    const static float C_FlipMatrix[4][3] = {
        { 1.0,  1.0,  1.0}, // Unflipped
        { 1.0, -1.0, -1.0},
        {-1.0,  1.0, -1.0},
        {-1.0, -1.0,  1.0}
    };
    const static unsigned int C_FlipMatrixSize = 
        sizeof(C_FlipMatrix) / sizeof(C_FlipMatrix[0]);

    MolFingerprinter::MolFingerprinter(PointList& hammsEllipsoidCoords,
        PointList& hammsSphereCoords, float epsilonSqr,
        unsigned int numFolds):
        m_axisAligner(hammsSphereCoords, epsilonSqr, true),
        m_volBox(hammsEllipsoidCoords, epsilonSqr),
        m_numFolds(numFolds), m_iFlip(0)
    {
    }

    MolFingerprinter::~MolFingerprinter() {
    }
    
    MolFingerprinter::MolFingerprinter(const MolFingerprinter& src):
        m_axisAligner(src.m_axisAligner),
        m_volBox(src.m_volBox),
        m_numFolds(src.m_numFolds),
        m_mol(src.m_mol), m_iFlip(src.m_iFlip),
        m_heavies(src.m_heavies)
    {
    }
    
    void MolFingerprinter::setMolecule(mol::Mol& mol)
    {
        m_mol = mol;
        m_iFlip = 0;
        m_heavies.clear();
        m_axisAligner.align_to_axes(m_mol);
        m_axisAligner.get_atom_points(m_mol.atoms(), m_heavies, false);
    }
    
    bool MolFingerprinter::getNextFP(BitVector& fp)
    {
        bool result(false);

        if (m_iFlip != C_FlipMatrixSize) {
            computeCurrFlipFingerprint(m_heavies, fp);
            m_iFlip++;
            result = true;
        }
        return result;
    }
    
    
    static inline void getFlippedPoints(
        const PointList& points, const float *flip, PointList& flippedPoints)
    {
        flippedPoints = points;
        
        PointList::iterator iEnd(flippedPoints.end());
        PointList::iterator i;
        for (i = flippedPoints.begin(); i != iEnd; ++i) {
            shape::Point& p(*i);
            p[0] *= flip[0];
            p[1] *= flip[1];
            p[2] *= flip[2];
        }
    }
    
    void MolFingerprinter::computeCurrFlipFingerprint(
        const PointList& points, BitVector& result)
    {
        const float *flip = C_FlipMatrix[m_iFlip];
        PointList flippedPoints;
        getFlippedPoints(points, flip, flippedPoints);
        
        result.resize(m_volBox.size() / (1 << m_numFolds));
        result.reset();
        m_volBox.set_folded_bits_for_spheres(
            flippedPoints, result, m_numFolds, 0);
    }
} // namespace mesaac

