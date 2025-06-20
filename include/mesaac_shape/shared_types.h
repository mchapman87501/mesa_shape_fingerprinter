// Defines types used throughout the mesaac_shape library.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved

#ifndef SHARED_TYPES_H_7FFXIUU6
#define SHARED_TYPES_H_7FFXIUU6

#include <vector>
#include "Globals.h"

namespace mesaac {
    namespace shape {
        // Informal: Point holds x, y, z and optional radius
        typedef std::vector<float> Point;
        typedef std::vector< Point > PointList;
        typedef BitVector Fingerprint;
        typedef ArrayBitVectors ShapeFingerprint;
    }
}


#endif /* end of include guard: SHARED_TYPES_H_7FFXIUU6 */
