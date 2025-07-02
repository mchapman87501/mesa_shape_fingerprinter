// Defines types used throughout the mesaac_shape library.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#pragma once

#include "mesaac_common/shape_defs.hpp"
#include <vector>

namespace mesaac {
namespace shape_eigen {
// Informal: Point holds x, y, z and optional radius
typedef std::vector<float> Point;
typedef std::vector<Point> PointList;
typedef shape_defs::BitVector Fingerprint;
typedef shape_defs::ArrayBitVectors ShapeFingerprint;
} // namespace shape_eigen
} // namespace mesaac
