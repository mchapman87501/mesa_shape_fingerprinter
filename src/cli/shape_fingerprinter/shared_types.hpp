// Provides types used throughout this app.
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved

#pragma once
#include <vector>

#include "mesaac_common/shape_defs.hpp"

namespace mesaac::shape_fingerprinter {
using FloatVector = std::vector<float>;
using PointList = std::vector<FloatVector>;
using ConformerPointsList = std::vector<PointList>;
} // namespace mesaac::shape_fingerprinter
