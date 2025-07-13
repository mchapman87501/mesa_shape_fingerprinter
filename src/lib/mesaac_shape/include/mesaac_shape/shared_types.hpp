// Defines types used throughout the mesaac_shape library.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#pragma once

#include "mesaac_common/shape_defs.hpp"
#include <vector>

namespace mesaac::shape {

/**
 * @brief A Point is a sequence of coordinates.  It typically holds x, y, z
 * coordinates.  In some uses it also holds a radius, e.g., for an Atom.
 */
using Point = std::vector<float>;

/**
 * @brief A vector of Points.
 */
using PointList = std::vector<Point>;

/**
 * @brief Represents a single shape (conformer) for a single orientation.
 */
using Fingerprint = shape_defs::BitVector;

/**
 * @brief A sequence of Fingerprints.
 */
using FingerprintVector = shape_defs::ArrayBitVectors;

/**
 * @brief A sequence of fingerprints for a single shape (conformer),
 * each of which represents the shape in a single "canonical" orientation.
 */
using ShapeFingerprint = shape_defs::ArrayBitVectors;

/**
 * @brief A ShapeFingerprintVector holds a collection of shape fingerprints.
 */
using ShapeFingerprintVector = shape_defs::ShapeFPBlocks;
} // namespace mesaac::shape
