//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/io/sdreader.hpp"

#include <sstream>

#include "mesaac_mol/element_info.hpp"

using namespace std;

namespace mesaac::mol {
SDReader::SDReader(istream &inf, string pathname)
    : m_pathname(pathname), m_inf(inf), m_linenum(0) {
  m_nums.imbue(locale("C"));
}

bool SDReader::getline(string &line) {
  std::getline(m_inf, line);
  bool result = (bool)m_inf;
  if (result) {
    m_linenum++;
  }
  // cerr << file_pos() << "'" << line << "'" << endl;
  return result;
}

string SDReader::file_pos() {
  ostringstream result;
  result << "File " << m_pathname << ", line " << m_linenum << ":  ";
  return result.str();
}

bool SDReader::double_field(string &line, unsigned int i_start,
                            unsigned int i_len, double &value) {
  m_nums.str(line.substr(i_start, i_len));
  m_nums.clear();
  m_nums >> value;
  return !m_nums.fail();
}

bool SDReader::uint_field(string &line, unsigned int i_start,
                          unsigned int i_len, unsigned int &value) {
  m_nums.str(line.substr(i_start, i_len));
  m_nums.clear();
  m_nums >> value;
  return !m_nums.fail();
}

// TODO:  Also extract number of atom lists, etc.
void SDReader::get_counts(unsigned int &num_atoms, unsigned int &num_bonds,
                          string &line) {
  num_atoms = 0;
  num_bonds = 0;
  if (getline(line)) {
    if (line.size() < 38) {
      cerr << file_pos() << "WARNING: SD counts line has incorrect length"
           << endl
           << "expected at least 38 characters, got " << line.size() << endl
           << "from line '" << line << "'" << endl;
    } else {
      if (!uint_field(line, 0, 3, num_atoms)) {
        cerr << file_pos() << "Could not read number of atoms "
             << "from '" << line << "'" << endl;
      }
      if (!uint_field(line, 3, 6, num_bonds)) {
        cerr << file_pos() << "Could not read number of bonds "
             << "from '" << line << "'" << endl;
      }
    }
  }
}

bool SDReader::read_next_atom(Atom &a) {
  bool result = false;
  string line;
  getline(line);
  // TODO: Enough w. the inline literal constants.
  if (line.size() < 34) {
    cerr << file_pos() << "Atom line is too short: '" << line << "'." << endl;
  } else {
    double x, y, z;
    if (double_field(line, 0, 10, x) && double_field(line, 10, 10, y) &&
        double_field(line, 20, 10, z)) {
      string symbol(line.substr(31, 3));
      string optional_cols(line.substr(34));
      a = Atom(get_atomic_num(symbol), {x, y, z}, optional_cols);
      result = true;
    } else {
      cerr << file_pos() << "Could not parse coordinates from '" << line << "'."
           << endl;
    }
  }
  return result;
}

bool SDReader::read_atoms(Mol &mol, unsigned int num_atoms) {
  bool result = true;
  for (unsigned int i = 0; i != num_atoms; i++) {
    Atom a;
    if (read_next_atom(a)) {
      mol.add_atom(a);
    } else {
      // When an error is encountered, give up on the current
      // molecule.
      result = false;
      break;
    }
  }
  return result;
}

bool SDReader::read_next_bond(Bond &b) {
  bool result = false;
  string line;
  getline(line);
  if (line.size() < 12) {
    cerr << file_pos() << "Warning: Bond line is too short (" << line.size()
         << " characters): '" << line << "'." << endl;
  }

  // Try to read anyway:
  {
    unsigned int a0, a1;
    Bond::BondTypeEnum bond_type;
    unsigned int uint_bond_type;
    Bond::BondStereoEnum stereo;
    unsigned int uint_stereo;
    if (uint_field(line, 0, 3, a0) && uint_field(line, 3, 6, a1) &&
        // So much for enum-driven value safety:
        uint_field(line, 6, 9, uint_bond_type) &&
        uint_field(line, 9, 12, uint_stereo)) {
      string optional_cols(line.substr(12));
      bond_type = (Bond::BondTypeEnum)uint_bond_type;
      stereo = (Bond::BondStereoEnum)uint_stereo;
      b = Bond(a0, a1, bond_type, stereo, optional_cols);
      result = true;
    } else {
      cerr << file_pos() << "Could not parse bond from '" << line << "'."
           << endl;
    }
  }
  return result;
}

bool SDReader::read_bonds(Mol &mol, unsigned int num_bonds) {
  bool result = true;
  for (unsigned int i = 0; i != num_bonds; i++) {
    Bond b;
    if (read_next_bond(b)) {
      mol.add_bond(b);
    } else {
      result = false;
      break;
    }
  }
  return result;
}

bool SDReader::read_properties_block(Mol &mol) {
  bool result = false;
  ostringstream blockf;
  string line;
  while (m_inf) {
    getline(line);
    blockf << line << endl;
    if (line == "M  END") {
      result = true;
      break;
    }
  }
  mol.properties_block(blockf.str());

  return result;
}

static bool is_blank(string &line) {
  return (line.find_first_not_of(" \t") == string::npos);
}

bool SDReader::read_one_tag(Mol &mol, string &line) {
  bool result = false;
  if (m_inf) {
    getline(line);
    if (line.substr(0, 1) == ">") {
      string tag = line;
      ostringstream value;
      while (m_inf) {
        getline(line);
        if (is_blank(line)) {
          break;
        } else {
          value << line << endl;
        }
      }
      // TODO:  Extract the actual tag, distinguishing between
      // <TAG_NAME>, DTn field numbers and registry numbers
      mol.add_unparsed_tag(tag, value.str());
      result = true;
    }
  }
  return result;
}

bool SDReader::read_tags(Mol &mol) {
  bool result = true;
  string line;
  while (read_one_tag(mol, line)) {
    // loop
  }
  if (line != "$$$$") {
    // TODO:  Exceptions
    cerr << file_pos() << "Did not find SD $$$$ delimiter." << endl;
    result = false;
  }
  return result;
}

void SDReader::skip_to_end() {
  while (m_inf) {
    string line;
    getline(line);
    if (line == "$$$$") {
      return;
    }
  }
}

bool SDReader::skip() {
  bool result = false;
  if (m_inf) {
    skip_to_end();
    result = true;
  }
  return result;
}

bool SDReader::read(Mol &next) {
  bool result = false;
  next.clear();
  if (m_inf) {
    // When to check for EOF?
    string mol_name;
    string mol_metadata;
    string comments;
    while (m_inf && !result) {
      unsigned int start_line(m_linenum);
      // If we can read the molecule name, assume we'll find a
      // complete molecule.
      if (getline(mol_name)) {
        getline(mol_metadata); // User's info, program name etc.
        getline(comments);     // comment
        next.name(mol_name);
        next.metadata(mol_metadata);
        next.comments(comments);

        unsigned int num_atoms = 0;
        unsigned int num_bonds = 0;
        string counts_line;
        get_counts(num_atoms, num_bonds, counts_line);
        next.counts_line(counts_line);

        // A molecule could be just an atom, I suppose, but it can't
        // be just bonds.
        if (num_atoms > 0) {
          // If unable to read this molecule, skip to the next one.
          result =
              (read_atoms(next, num_atoms) && read_bonds(next, num_bonds) &&
               read_properties_block(next) && read_tags(next));
        }
        if (!result) {
          cerr << "Could not read molecule starting at line " << start_line
               << "; skipping ahead." << endl;
          skip_to_end();
        }
      }
    }
  }
  return result;
}
} // namespace mesaac::mol
