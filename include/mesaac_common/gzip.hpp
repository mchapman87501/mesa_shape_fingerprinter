//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>

namespace mesaac::common::gzip {

/// @brief Compress a string.
/// @param src string to compress
/// @param level desired compression level
/// @return the compressed representation of `src`
std::string compress(const std::string src, int level = 8);

/// @brief Decompress gzip-compressed data.
/// @param src data to decompress
/// @return the decompressed data from `src`
std::string decompress(const std::string src);

} // namespace mesaac::common::gzip
