// Defines the entry points to the feature_finder library.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "i_feature_finder.hpp"
#include <string>

IFeatureFinder *create_feature_finder(std::string sdf);
