//
// Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>

namespace mesaac {
namespace common {
class B32 {
public:
  B32() {}

  std::string encode(std::string src);
  std::string decode(std::string src);
};
} // namespace common
} // namespace mesaac
