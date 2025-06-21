//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/mol.h"

#include <iostream>
#include <sstream>

using namespace std;

namespace mesaac {
    namespace mol {
        Mol::Mol() {
        }

        static void clear_atoms(AtomVector& atoms) {
            while (atoms.size() > 0) {
                Atom *last = atoms.back();
                atoms.pop_back();
                delete last;
            }
        }
        
        static void clear_bonds(BondVector& bonds) {
            while (bonds.size() > 0) {
                Bond *last = bonds.back();
                bonds.pop_back();
                delete last;
            }
        }
        
        Mol::~Mol() {
            clear_atoms(m_atoms);
            clear_bonds(m_bonds);
        }
        
        Mol::Mol(const Mol& src) {
            *this = src;
        }
        
        Mol& Mol::operator=(const Mol& src) {
            m_name = src.m_name;
            m_metadata = src.m_metadata;
            m_comments = src.m_comments;
            m_counts_line = src.m_counts_line;
            // TODO:  Share ref-counted atoms and bonds?
            {
                clear_atoms(m_atoms);
                const AtomVector& src_atoms(src.m_atoms);
                AtomVector::const_iterator i;
                for (i = src_atoms.begin(); i != src_atoms.end(); i++) {
                    Atom *a = new Atom(**i);
                    m_atoms.push_back(a);
                }
            }
            {
                clear_bonds(m_bonds);
                const BondVector& src_bonds(src.m_bonds);
                BondVector::const_iterator i;
                for (i = src_bonds.begin(); i != src_bonds.end(); i++) {
                    Bond *b = new Bond(**i);
                    m_bonds.push_back(b);
                }
            }
            m_tags = src.m_tags;
            m_properties_block = src.m_properties_block;
            return *this;
        }
    
        void Mol::clear() {
            m_name = "";
            m_metadata = "";
            m_comments = "";
            clear_atoms(m_atoms);
            clear_bonds(m_bonds);
            m_tags.clear();
            m_properties_block = "";
        }
    
        void Mol::name(const string& new_value) {
            m_name = new_value;
        }
    
        void Mol::metadata(const string& new_value) {
            m_metadata = new_value;
        }
    
        void Mol::comments(const string& new_value) {
            m_comments = new_value;
        }
    
        void Mol::add_atom(Atom& atom) {
            m_atoms.push_back(new Atom(atom));
        }
    
        void Mol::add_bond(Bond& bond) {
            m_bonds.push_back(new Bond(bond));
        }
    
        void Mol::properties_block(const string& new_value) {
            m_properties_block = new_value;
        }
    
        void Mol::add_unparsed_tag(const string& tag_line, const string& value) {
            if (m_tags.find(tag_line) != m_tags.end()) {
                // TODO:  Throw exception
                cerr << "Warning: molecule already has tag '" << tag_line 
                     << "'.  Overwriting with new value." << endl;
            }
            m_tags[tag_line] = value;
        }
    
        void Mol::add_tag(const string& name, const string& value) {
            string tag = ">  <" + name + ">";
            add_unparsed_tag(tag, value);
        }
    
        // void Mol::add_tag(const string& name, int value) {
        //     ostringstream outs;
        //     outs << value << endl; // Tags should be terminated by a blank line.
        //     add_tag(name, outs.str());
        // }
    
        unsigned int Mol::num_atoms() {
            return m_atoms.size();
        }
    
        unsigned int Mol::num_heavy_atoms() {
            unsigned int result = 0;
            AtomVector::iterator i;
            for (i = m_atoms.begin(); i != m_atoms.end(); ++i) {
                Atom *a(*i);
                if (a && !a->is_hydrogen()) {
                    result++;
                }
            }
            return result;
        }

        // Based on a layman's reading of MDL ctfile spec:
        // Returns 2 if all z coordinates are zero, 3 otherwise.
        unsigned int Mol::dimensionality() {
            // TODO:  Cache this value, invalidating whenever atoms are added or
            // cleared.
            AtomVector::iterator i;
            for (i = m_atoms.begin(); i != m_atoms.end(); ++i) {
                Atom *a(*i);
                if (a && (0 != a->z())) {
                    return 3;
                }
            }
            return 2;
        }
    } // namespace mol
} // namespace mesaac

