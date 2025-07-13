// Provides types used throughout this app.
// Copyright (c) 2005-2009 Mesa Analytics & Computing, Inc.  All rights reserved

#pragma once

#include <vector>

#include "mesaac_measures/measures_base.hpp"
#include "mesaac_shape/shared_types.hpp"

namespace mesaac::align_monte {
typedef shape::Point FloatVector;
typedef shape::PointList PointList;
typedef std::vector<PointList> ConformerPointsList;

typedef std::vector<measures::MeasuresBase::Ptr> MeasuresList;
} // namespace mesaac::align_monte
