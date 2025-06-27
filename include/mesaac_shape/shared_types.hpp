// Defines types used throughout the mesaac_shape library.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#pragma once

#include "shape_defs.hpp"
#include <vector>

namespace mesaac {
namespace shape {
// Informal: Point holds x, y, z and optional radius
using Point = std::vector<float>;
using PointList = std::vector<Point>;
using Fingerprint = shape_defs::BitVector;
using ShapeFingerprint = shape_defs::ArrayBitVectors;
} // namespace shape
} // namespace mesaac
