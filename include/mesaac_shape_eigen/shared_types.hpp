// Defines types used throughout the mesaac_shape library.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#ifndef SHARED_TYPES_H_7FFXIUU6
#define SHARED_TYPES_H_7FFXIUU6

#include "shape_defs.hpp"
#include <vector>

namespace mesaac {
namespace shape_eigen {
// Informal: Point holds x, y, z and optional radius
typedef std::vector<float> Point;
typedef std::vector<Point> PointList;
typedef BitVector Fingerprint;
typedef ArrayBitVectors ShapeFingerprint;
} // namespace shape_eigen
} // namespace mesaac

#endif /* end of include guard: SHARED_TYPES_H_7FFXIUU6 */
