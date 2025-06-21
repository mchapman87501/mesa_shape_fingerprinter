//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/bond.h"

using namespace std;

namespace mesaac {
    namespace mol {
        Bond::Bond():
            m_a0(0), m_a1(0), m_type(BTE_SINGLE), m_stereo(BSE_NOT_STEREO),
            m_optional_cols("")
        {
        }
    
        Bond::Bond(const Bond& src) {
            (*this) = src;
        }
    
        Bond& Bond::operator=(const Bond& src) {
            m_a0 = src.m_a0;
            m_a1 = src.m_a1;
            m_type = src.m_type;
            m_stereo = src.m_stereo;
            m_optional_cols = src.m_optional_cols;
            return *this;
        }

        Bond::~Bond() {
        }
    } // namespace mol
} // namespace mesaac

