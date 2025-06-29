//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "atom.hpp"
#include "bond.hpp"
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace mesaac::mol {
typedef std::map<std::string, std::string> SDTagMap;

class Mol {
public:
  Mol() {}

  void clear();

  void name(const std::string &new_value);
  void metadata(const std::string &new_value);
  void comments(const std::string &new_value);
  void counts_line(const std::string &new_value) { m_counts_line = new_value; }

  void add_atom(Atom &a);
  void add_bond(Bond &b);

  void properties_block(const std::string &new_value);

  void add_unparsed_tag(const std::string &tag_line, const std::string &value);
  void add_tag(const std::string &name, const std::string &value);
  template <typename T> void add_tag(const std::string &name, const T &value) {
    std::ostringstream outs;
    outs << value;
    add_tag(name, outs.str());
  }
  // void add_tag(const std::string& name, int value);

  std::string name() { return m_name; }
  std::string metadata() { return m_metadata; }
  std::string comments() { return m_comments; }
  std::string counts_line() { return m_counts_line; }

  unsigned int num_atoms();
  unsigned int num_heavy_atoms();

  // Dimensionality of the atom coordinate data.
  // Based on a layman's reading of MDL ctfile spec:
  // Returns 2 if all z coordinates are zero, 3 otherwise.
  unsigned int dimensionality();

  const AtomVector &atoms() { return m_atoms; }
  AtomVector &mutable_atoms() { return m_atoms; }

  const BondVector &bonds() { return m_bonds; }

  std::string properties_block() { return m_properties_block; }
  const SDTagMap &tags() const { return m_tags; }

protected:
  std::string m_name;
  std::string m_metadata;
  std::string m_comments;
  std::string m_counts_line;

  AtomVector m_atoms;
  BondVector m_bonds;

  std::string m_properties_block;
  SDTagMap m_tags;
};
} // namespace mesaac::mol
