//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <string>
#include <vector>

namespace mesaac::mol {
class Bond {
public:
  typedef enum {
    BTE_SINGLE = 1,
    BTE_DOUBLE = 2,
    BTE_TRIPLE = 3,
    BTE_AROMATIC = 4,
    BTE_SINGLE_OR_DOUBLE = 5,
    BTE_SINGLE_OR_AROMATIC = 6,
    BTE_DOUBLE_OR_AROMATIC = 7,
    BTE_ANY = 8
  } BondTypeEnum;

  typedef enum {
    BSE_NOT_STEREO = 0,
    BSE_UP = 1,
    BSE_EITHER = 4,
    BSE_DOWN_DOUBLE = 6, // ?
    BSE_CIS_TRANS_DOUBLE = 3
  } BondStereoEnum;

  Bond()
      : m_a0(0), m_a1(0), m_type(BTE_SINGLE), m_stereo(BSE_NOT_STEREO),
        m_optional_cols("") {}

  Bond(unsigned int a0, unsigned int a1, BondTypeEnum bond_type = BTE_SINGLE,
       BondStereoEnum stereo = BSE_NOT_STEREO,
       const std::string &optional_cols = "")
      : m_a0(a0), m_a1(a1), m_type(bond_type), m_stereo(stereo),
        m_optional_cols(optional_cols) {}

  unsigned int a0() const { return m_a0; }
  unsigned int a1() const { return m_a1; }
  BondTypeEnum type() const { return m_type; }
  BondStereoEnum stereo() const { return m_stereo; }
  std::string optional_cols() const { return m_optional_cols; }

protected:
  unsigned int m_a0, m_a1;
  BondTypeEnum m_type;
  BondStereoEnum m_stereo;
  std::string m_optional_cols;
};

typedef std::vector<Bond> BondVector;
} // namespace mesaac::mol
