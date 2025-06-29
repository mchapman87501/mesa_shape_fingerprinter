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
  virtual ~SDWriter();

  bool write(Mol &mol);

protected:
  std::ostream &m_outf;
  std::ostringstream m_fmt;

  std::string f_str(float f, const unsigned int field_width = 10,
                    unsigned int decimals = 4);
  std::string uint_str(unsigned int u, const unsigned int field_width = 3);

private:
  SDWriter(const SDWriter &src);
  SDWriter &operator=(const SDWriter &src);
};
} // namespace mesaac::mol
