// Provides types used throughout this app.
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved

#ifndef SHARED_TYPES_H_N816I3BW
#define SHARED_TYPES_H_N816I3BW

#include <vector>

#include "shape_defs.hpp"

namespace mesaac {
typedef std::vector<float> FloatVector;
typedef std::vector<FloatVector> PointList;
typedef std::vector<PointList> ConformerPointsList;
} // namespace mesaac

#endif /* end of include guard: SHARED_TYPES_H_N816I3BW */
