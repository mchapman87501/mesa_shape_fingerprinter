//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_common/b32.hpp"

namespace mesaac::common {

/// @brief A Base64 codec
class B64 {
public:
  B64() {}

  /// @brief Encode data as a B64 string.
  /// @param src data to encode
  /// @return The B64 representation of src
  std::string encode(std::string src);

  /// @brief Decode a B64 string.
  /// @param src B64 string to decode
  /// @return The decoded representation of src
  std::string decode(std::string src);
};
} // namespace mesaac::common
