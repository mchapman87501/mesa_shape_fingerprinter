//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol.hpp"
#include <iostream>
#include <sstream>

namespace mesaac::mol {
class SDWriter {
public:
  SDWriter(std::ostream &outf);

  bool write(Mol &mol);

protected:
  std::ostream &m_outf;

private:
  SDWriter(const SDWriter &src);
  SDWriter(SDWriter &&src);
  SDWriter &operator=(const SDWriter &src);
  SDWriter &operator=(const SDWriter &&src);
};
} // namespace mesaac::mol
