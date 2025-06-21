// Defines types used throughout the mesaac_shape library.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#pragma once

#include "shape_defs.hpp"
#include <vector>

namespace mesaac {
namespace shape {
// Informal: Point holds x, y, z and optional radius
typedef std::vector<float> Point;
typedef std::vector<Point> PointList;
typedef BitVector Fingerprint;
typedef ArrayBitVectors ShapeFingerprint;
} // namespace shape
} // namespace mesaac
