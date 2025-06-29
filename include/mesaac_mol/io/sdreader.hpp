//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#pragma once

#include "mesaac_mol/mol.hpp"
#include <iostream>
#include <sstream>
#include <string>

namespace mesaac::mol {

class SDReader {
public:
  SDReader(std::istream &inf, std::string pathname = "(input stream)");

  bool skip();
  bool read(Mol &mol);

protected:
  std::string m_pathname;
  std::istream &m_inf;
  unsigned int m_linenum;

  std::istringstream m_nums;

  std::string file_pos();
  bool double_field(std::string &line, unsigned int i_start, unsigned int i_len,
                    double &value);
  bool uint_field(std::string &line, unsigned int i_start, unsigned int i_len,
                  unsigned int &value);

  bool getline(std::string &line);
  void get_counts(unsigned int &num_atoms, unsigned int &num_bonds,
                  std::string &line);
  bool read_next_atom(Atom &a);
  bool read_atoms(Mol &mol, unsigned int num_atoms);

  bool read_next_bond(Bond &b);
  bool read_bonds(Mol &mol, unsigned int num_bonds);

  bool read_properties_block(Mol &mol);
  bool read_one_tag(Mol &mol, std::string &line);
  bool read_tags(Mol &mol);

  void skip_to_end();

private:
  SDReader(const SDReader &src);
  SDReader &operator=(const SDReader &src);
};
} // namespace mesaac::mol
