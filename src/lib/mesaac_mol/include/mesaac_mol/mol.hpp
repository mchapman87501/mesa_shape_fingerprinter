//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol/atom.hpp"
#include "mesaac_mol/bond.hpp"
#include "mesaac_mol/sd_tag_map.hpp"

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace mesaac::mol {

class Mol {
public:
  struct MolParams {
    const AtomVector atoms;
    const BondVector bonds;
    const SDTagMap tags;

    const std::string name;
    const std::string metadata;
    const std::string comments;
    const std::string counts_line;
    const std::string properties_block;
  };

  Mol() {}
  Mol(const MolParams &&params)
      : m_atoms(params.atoms), m_bonds(params.bonds), m_tags(params.tags),
        m_name(params.name), m_metadata(params.metadata),
        m_comments(params.comments), m_counts_line(params.counts_line),
        m_properties_block(params.properties_block) {}

  std::string name() const { return m_name; }
  std::string metadata() const { return m_metadata; }
  std::string comments() const { return m_comments; }
  std::string counts_line() const { return m_counts_line; }

  unsigned int num_atoms() const { return m_atoms.size(); }
  unsigned int num_heavy_atoms() const;

  // Dimensionality of the atom coordinate data.
  // Based on a layman's reading of MDL ctfile spec:
  // Returns 2 if all z coordinates are zero, 3 otherwise.
  unsigned int dimensionality() const;

  const AtomVector &atoms() const { return m_atoms; }

  // A sop to C++-Swift interop:
  void get_atom(unsigned int index, Atom &result) const {
    result = m_atoms.at(index);
  }

  AtomVector &mutable_atoms() { return m_atoms; }

  unsigned int num_bonds() const { return m_bonds.size(); }
  const BondVector &bonds() const { return m_bonds; }

  // Sop, continued:
  void get_bond(unsigned int index, Bond &result) const {
    result = m_bonds.at(index);
  }

  std::string properties_block() const { return m_properties_block; }
  const SDTagMap &tags() const { return m_tags; }
  SDTagMap &mutable_tags() { return m_tags; }

protected:
  AtomVector m_atoms;
  BondVector m_bonds;
  SDTagMap m_tags;

  std::string m_name;
  std::string m_metadata;
  std::string m_comments;
  std::string m_counts_line;

  std::string m_properties_block;
};
} // namespace mesaac::mol
