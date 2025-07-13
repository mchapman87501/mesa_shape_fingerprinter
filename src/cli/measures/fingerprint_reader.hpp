//
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>

#include "mesaac_common/shape_defs.hpp"
namespace mesaac::cli::measures {
// Read fingerprints from the named file, returning them in fingerprints.
// If pathname is '-', read from stdin.
void read_fingerprints(const std::string &pathname,
                       shape_defs::ArrayBitVectors &fingerprints);
} // namespace mesaac::cli::measures
