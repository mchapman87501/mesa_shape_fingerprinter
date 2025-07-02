//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol/mol.hpp"
#include <iostream>
#include <optional>
#include <sstream>
#include <string>

namespace mesaac::mol {

/// @brief SDReader reads Mol instances from an SD file.
class SDReader {
public:
  /// @brief Create an SD file reader.
  /// @param inf stream from which to read SD structures
  /// @param pathname if given, the pathname from which `inf` is reading
  SDReader(std::istream &inf, const std::string &pathname = "(input stream)");

  /// @brief Skip the next molecule.
  /// @return true if the reader was able to skip ahead
  bool skip();

  /// @brief Read the next molecule/structure.
  /// @param mol on successful return, the next read molecule
  /// @return whether or not a molecule could be read into `mol`
  ///
  /// @note If the return value is `false`, then `mol` will be an empty
  /// molecule.
  bool read(Mol &mol);

protected:
  std::string m_pathname;
  std::istream &m_inf;
  unsigned int m_linenum;

  std::string file_pos();

  bool getline(std::string &line);
  void get_counts(unsigned int &num_atoms, unsigned int &num_bonds,
                  std::string &line);
  std::optional<Atom> read_next_atom();
  bool read_atoms(AtomVector &atoms, unsigned int num_atoms);

  std::optional<Bond> read_next_bond();
  bool read_bonds(BondVector &bonds, unsigned int num_bonds);

  bool read_properties_block(std::string &properties_block);
  bool read_one_tag(SDTagMap &tags, std::string &line);
  bool read_tags(SDTagMap &tags);

  void skip_to_end();

private:
  SDReader(const SDReader &src);
  SDReader &operator=(const SDReader &src);
};
} // namespace mesaac::mol
