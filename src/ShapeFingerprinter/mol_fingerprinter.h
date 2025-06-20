// 
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved
//

#ifndef _MOL_FINGERPRINTER4B083DD6_H_
#define _MOL_FINGERPRINTER4B083DD6_H_

// Singular value decomposition, for PCA -- this defines ap::real_2d_array
#include "svd.h"
#include "shared_types.h"
#include "mesaac_mol.h"
#include "mesaac_shape.h"

namespace mesaac {
    class MolFingerprinter {
    public:
        MolFingerprinter(PointList& hammsEllipsoidCoords, 
            PointList& hammsSphereCoords, float epsilonSqr, 
            unsigned int numFolds);
        virtual ~MolFingerprinter();

        explicit MolFingerprinter(const MolFingerprinter& src);
    
        void setMolecule(mol::Mol& mol);
        bool getNextFP(BitVector& fp);
        
    protected:
        shape::AxisAligner m_axisAligner;
        shape::VolBox m_volBox;
        unsigned int m_numFolds;
        
        mol::Mol m_mol;
        unsigned int m_iFlip;
        PointList m_heavies;

        void computeCurrFlipFingerprint(const PointList& points, 
                                        BitVector& result);
        
    private:
        MolFingerprinter& operator=(const MolFingerprinter& src);
        
    };
} // namespace mesaac
#endif // _MOL_FINGERPRINTER4B083DD6_H_
