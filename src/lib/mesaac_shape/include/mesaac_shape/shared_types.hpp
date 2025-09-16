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
 * @brief Indicates which points in a point intersect with a single orientation
 * of a conformer.
 * @details A Fingerprint is a sequence of bits, each corresponding to a single
 * point in a quasi-random point set.  A Fingerprint is meant to represent a
 * conformer in a single spatial orientation.  In such a Fingerprint, a bit is
 * non-zero if the corresponding point intersects the volume of the conformer.
 */
using Fingerprint = shape_defs::BitVector;

/**
 * @brief A sequence of Fingerprints.
 */
using FingerprintVector = shape_defs::ArrayBitVectors;

/**
 * @brief A sequence of Fingerprints for a single shape (conformer),
 * each of which represents the shape in a single "canonical" orientation.
 */
using ShapeFingerprint = shape_defs::ArrayBitVectors;

/**
 * @brief A collection of ShapeFingerprints.
 */
using ShapeFingerprintVector = shape_defs::ShapeFPBlocks;
} // namespace mesaac::shape
