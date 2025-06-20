// Do-nothing implementation of feature_finder_factory interface.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "shape_feature_fingerprinter.h"

class StubFeatureFinder: public IFeatureFinder {
public:
    StubFeatureFinder() {}
    virtual ~StubFeatureFinder() {}
    
    virtual unsigned int num_feature_types() {
        return 0;
    }
    
    virtual void get_feature_atom_indices(
        unsigned int /* feature_index */, IntVector& indices) 
    {
        indices.clear();
    }
};

IFeatureFinder *create_feature_finder(std::string /* sdf */) {
    return new StubFeatureFinder();
}
