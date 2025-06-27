//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>

namespace mesaac {
namespace common {
namespace gzip {
std::string compress(const std::string src, int level = 8);
std::string decompress(const std::string src);

} // namespace gzip
} // namespace common
} // namespace mesaac
