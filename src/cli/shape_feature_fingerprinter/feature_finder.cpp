//
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "feature_finder.h"

using namespace std;

#include <vector>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <openbabel/mol.h>
#include <openbabel/parsmart.h>
#include <openbabel/obconversion.h>
#include "feature_finder.h"

using namespace std;
using namespace OpenBabel;

static const char *C_feature_smarts[] = {
    "[#6,#7;R0]=[#8]", // "HBond-Acceptor"
    "[O,N,S;!H0;R0]",  // "HBond-Donor"
    "[+1,+2,-1,-2]",   // "Charged"
    "a1aaa1",          // "4-Member Aromatic Ring"
    "a1aaaa1",         // "5-Member Aromatic Ring"
    "a1aaaaa1",        // "6-Member Aromatic Ring"
};
static unsigned int C_num_feature_smarts = 
    sizeof(C_feature_smarts) / sizeof(C_feature_smarts[0]);

typedef vector<OBSmartsPattern> SmartsPatternVector;
static SmartsPatternVector s_smarts;

static void init_s_smarts() {
    if (s_smarts.size() == 0) {
        for (unsigned int i = 0; i != C_num_feature_smarts; ++i) {
            string smarts_str(C_feature_smarts[i]);
            OBSmartsPattern p;
            if (!p.Init(smarts_str)) {
                cerr << "Could not interpret SMARTS string '" 
                     << smarts_str << "'" << endl;
                exit(1);
            }
            s_smarts.push_back(p);
        }
    }
}

namespace mesaac {

    FeatureFinder::FeatureFinder(string sdf) {
        m_mol = new OBMol();
        // In OpenBabel 2.2.3, OBConversion leaks stream filters 
        // installed as a side effect of invoking ReadString.  Workaround:
        // one shared, thread-unsafe instance of OBConversion.
        static OBConversion conv;
        if (!conv.SetInFormat("sdf")) {
            throw runtime_error("Could not initialize OBConversion for SD input format.");
        }
        if (!conv.ReadString(m_mol, sdf)) {
            ostringstream msg;
            msg << "Could not read molecule from SD string:" << endl
                << sdf;
            throw runtime_error(msg.str());
        }
    }

    FeatureFinder::~FeatureFinder() {
        delete m_mol;
    }

    unsigned int FeatureFinder::num_feature_types() {
        init_s_smarts();
        return s_smarts.size();
    }
        
    // Get the zero-based indices of the atoms of mol which match
    // the indexed feature.  Feature indices are in the range 0..
    // num_feature_types() - 1, inclusive.
    void FeatureFinder::get_feature_atom_indices(
        unsigned int feature_index, IntVector& indices)
    {
        if (feature_index >= C_num_feature_smarts) {
            throw range_error("feature_index is out of range");
        }
        
        if (m_mol_features.size() == 0) {
            // Pre-compute all feature atom lists, on assumption all
            // will be retrieved.
            compute_feature_atom_lists();
        }
        indices.clear();
        IntVector& feature_indices(m_mol_features.at(feature_index));
        indices = feature_indices;
    }
    
    void FeatureFinder::compute_feature_atom_lists() {
        init_s_smarts();
        for (unsigned int i = 0; i != C_num_feature_smarts; ++i) {
            OBSmartsPattern& pattern(s_smarts[i]);
            IntVector indices; // Atom indices for current pattern.
            if (pattern.Match(*m_mol)) {
                indices.reserve(pattern.NumMatches());
                typedef vector<IntVector> MatchList;
                // Flatten all of the unique matches into a single list 
                // of atom indices.
                MatchList umatches = pattern.GetUMapList();
                MatchList::iterator j;
                for (j = umatches.begin(); j != umatches.end(); ++j) {
                    IntVector& curr_match(*j);
                    IntVector::iterator k;
                    for (k = curr_match.begin(); k != curr_match.end(); ++k) {
                        // XXX FIX THIS are these OB indices zero-based
                        // or 1-based?
                        unsigned int i_atom = (*k) - 1;
                        indices.push_back(i_atom);
                    }
                }
            }
            m_mol_features.push_back(indices);
        }
    }

} // namespace mesaac
