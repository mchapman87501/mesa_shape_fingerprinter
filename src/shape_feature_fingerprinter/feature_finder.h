//
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include <openbabel/mol.h>
#include <string>

#include "shape_feature_fingerprinter/i_feature_finder.hpp"

namespace mesaac {
typedef std::vector<IFeatureFinder::IntVector> FeatureAtomLists;

class FeatureFinder : public IFeatureFinder {
public:
  FeatureFinder(std::string sdf);
  virtual ~FeatureFinder();

  virtual unsigned int num_feature_types();
  virtual void get_feature_atom_indices(unsigned int feature_index,
                                        IntVector &indices);

protected:
  OpenBabel::OBMol *m_mol;
  FeatureAtomLists m_mol_features;

  void compute_feature_atom_lists();

private:
  FeatureFinder(const FeatureFinder &src);
  FeatureFinder &operator=(const FeatureFinder &src);
};

} // namespace mesaac
