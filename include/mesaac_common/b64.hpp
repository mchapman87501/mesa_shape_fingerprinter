//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_common/b32.hpp"

namespace mesaac {
namespace common {
class B64 {
public:
  B64();
  virtual ~B64();

  std::string encode(std::string src);
  std::string decode(std::string src);
};
} // namespace common
} // namespace mesaac
