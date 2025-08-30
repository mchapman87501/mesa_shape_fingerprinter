// Provides types used throughout this app.
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved

#pragma once

#include <vector>

#include "mesaac_measures/measures_base.hpp"
#include "mesaac_shape/shared_types.hpp"

namespace mesaac::align_monte {
using ConformerPointsList = std::vector<shape::Point3DList>;
using MeasuresList = std::vector<measures::MeasuresBase::Ptr>;
} // namespace mesaac::align_monte
