//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/atom.h"
#include "mesaac_mol/element_info.h"
//#include <iostream>

using namespace std;

namespace mesaac {
    namespace mol {
        // void update_extant(int delta) {
        //     static unsigned int cnt = 0;
        //     cnt += delta;
        //     cout << "Atoms: " << cnt << endl;
        // }
        
        Atom::Atom(): m_atomic_num(0), m_x(0), m_y(0), m_z(0) {
            // update_extant(1);
        }
    
        Atom::Atom(const Atom& src)
        {
            (*this) = src;
            // update_extant(1);
        }

        Atom::~Atom() {
            // update_extant(-1);
        }
    
        Atom& Atom::operator=(const Atom& src) {
            m_atomic_num = src.m_atomic_num;
            m_x = src.m_x;
            m_y = src.m_y;
            m_z = src.m_z;
            m_optional_cols = src.m_optional_cols;
            return *this;
        }

        void Atom::atomic_num(unsigned int new_value) {
            m_atomic_num = new_value;
        }
    
        void Atom::x(float new_value) {
            m_x = new_value;
        }
    
        void Atom::y(float new_value) {
            m_y = new_value;
        }
    
        void Atom::z(float new_value) {
            m_z = new_value;
        }
    
        void Atom::optional_cols(const string& new_value) {
            m_optional_cols = new_value;
        }

        string Atom::symbol() const {
            return getSymbol(m_atomic_num);
        }
    
        float Atom::radius() const {
            return getRadius(m_atomic_num);
        }
    
        bool Atom::is_hydrogen() const {
            bool result = (1 == m_atomic_num);
            return result;
        }
    } // namespace mol
} // namespace mesaac

