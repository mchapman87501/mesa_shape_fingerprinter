//
// Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>

namespace mesaac::common {

/// @brief A Base32 codec
class B32 {
public:
  B32() {}

  /// @brief Encode data as a B32 string.
  /// @param src data to encode
  /// @return The B32 representation of `src`
  std::string encode(std::string src) const;

  /// @brief Decode a B32 string.
  /// @param src B32 string to decode
  /// @return The decoded representation of `src`
  std::string decode(std::string src) const;
};
} // namespace mesaac::common
