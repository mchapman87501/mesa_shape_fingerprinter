// Defines types used throughout the mesaac_shape library.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#pragma once

#include "mesaac_common/shape_defs.hpp"
#include <array>
#include <vector>
namespace mesaac::shape {

/**
 * @brief A Point3D is a location in 3-space: {x, y, z}
 */
using Point3D = std::array<float, 3>;

/**
 * @brief A Sphere is represented as a location and a radius: {x, y, z, radius}
 */
using Sphere = std::array<float, 4>;

using Point3DList = std::vector<Point3D>;
using SphereList = std::vector<Sphere>;

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
