//
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "shape_feature_fingerprinter.h"
#include "feature_finder.h"

IFeatureFinder *create_feature_finder(std::string sdf) {
    return new mesaac::FeatureFinder(sdf);
}
