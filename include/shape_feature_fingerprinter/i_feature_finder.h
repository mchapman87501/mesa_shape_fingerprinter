// Defines the public interface of a feature finder.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#ifndef I_FEATURE_FINDER_H_BW2FMVE9
#define I_FEATURE_FINDER_H_BW2FMVE9

#include <vector>

class IFeatureFinder {
public:
    typedef std::vector<int> IntVector;
    
    virtual ~IFeatureFinder() {}
    
    virtual unsigned int num_feature_types() = 0;
    // Get zero-based indices of all atoms which match the
    // zero-based feature index.
    virtual void get_feature_atom_indices(
        unsigned int feature_index, IntVector& indices) = 0;
};


#endif /* end of include guard: I_FEATURE_FINDER_H_BW2FMVE9 */
