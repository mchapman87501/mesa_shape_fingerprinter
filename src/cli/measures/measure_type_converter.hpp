#pragma once

#include "mesaac_measures/measures_factory.hpp"

namespace mesaac::cli::measures {
mesaac::measures::MeasureType get_measure_type(const char type_code);
}