//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/io/sdreader.hpp"
#include "mesaac_mol/element_info.hpp"

#include <sstream>

using namespace std;

namespace mesaac::mol {

namespace {
bool double_field(const string &line, unsigned int i_start, unsigned int i_len,
                  double &value) {
  try {
    value = std::stod(line.substr(i_start, i_len));
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

bool uint_field(const string &line, unsigned int i_start, unsigned int i_len,
                unsigned int &value) {
  try {
    value = std::stoul(line.substr(i_start, i_len));
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

} // namespace

SDReader::SDReader(istream &inf, const string &pathname)
    : m_pathname(pathname), m_inf(inf), m_linenum(0) {}

bool SDReader::getline(string &line) {
  std::getline(m_inf, line);
  bool result = m_inf.good();
  if (result) {
    m_linenum++;
  }
  return result;
}

string SDReader::file_pos() {
  ostringstream result;
  result << "File " << m_pathname << ", line " << m_linenum << ":  ";
  return result.str();
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

std::optional<Atom> SDReader::read_next_atom() {
  string line;
  getline(line);
  // TODO: Enough w. the inline literal constants.
  if (line.size() < 34) {
    cerr << file_pos() << "Atom line is too short: '" << line << "'." << endl;
  } else {
    double x, y, z;
    if (double_field(line, 0, 10, x) && double_field(line, 10, 10, y) &&
        double_field(line, 20, 10, z)) {
      return Atom({.atomic_num = get_atomic_num(line.substr(31, 3)),
                   .pos = {x, y, z},
                   .optional_cols = line.substr(34)});
    } else {
      cerr << file_pos() << "Could not parse coordinates from '" << line << "'."
           << endl;
    }
  }
  return std::nullopt;
}

bool SDReader::read_atoms(AtomVector &atoms, unsigned int num_atoms) {
  for (unsigned int i = 0; i != num_atoms; i++) {
    const auto atom = read_next_atom();
    if (atom.has_value()) {
      atoms.emplace_back(atom.value());
    } else {
      // When an error is encountered, give up on the current
      // molecule.
      return false;
    }
  }
  return true;
}

std::optional<Bond> SDReader::read_next_bond() {
  string line;
  getline(line);
  if (line.size() < 12) {
    cerr << file_pos() << "Warning: Bond line is too short (" << line.size()
         << " characters): '" << line << "'." << endl;
  }

  // Try to read anyway:
  {
    unsigned int a0, a1;
    BondType bond_type;
    unsigned int uint_bond_type;
    BondStereo stereo;
    unsigned int uint_stereo;
    if (uint_field(line, 0, 3, a0) && uint_field(line, 3, 6, a1) &&
        // So much for enum-driven value safety:
        uint_field(line, 6, 9, uint_bond_type) &&
        uint_field(line, 9, 12, uint_stereo)) {
      string optional_cols(line.substr(12));
      bond_type = static_cast<BondType>(uint_bond_type);
      stereo = static_cast<BondStereo>(uint_stereo);
      return Bond({a0, a1, bond_type, stereo, optional_cols});
    } else {
      cerr << file_pos() << "Could not parse bond from '" << line << "'."
           << endl;
    }
  }
  return std::nullopt;
}

bool SDReader::read_bonds(BondVector &bonds, unsigned int num_bonds) {
  for (unsigned int i = 0; i != num_bonds; i++) {
    const auto bond = read_next_bond();
    if (bond.has_value()) {
      bonds.emplace_back(bond.value());
    } else {
      return false;
    }
  }
  return true;
}

bool SDReader::read_properties_block(string &properties_block) {
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
  properties_block = blockf.str();

  return result;
}

static bool is_blank(string &line) {
  return (line.find_first_not_of(" \t") == string::npos);
}

bool SDReader::read_one_tag(SDTagMap &tags, string &line) {
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
      tags.add_unparsed(tag, value.str());
      result = true;
    }
  }
  return result;
}

bool SDReader::read_tags(SDTagMap &tags) {
  bool result = true;
  string line;
  while (read_one_tag(tags, line)) {
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
  string name;
  string metadata;
  string comments;
  AtomVector atoms;
  BondVector bonds;
  string properties_block;
  SDTagMap tags;
  string counts_line;

  bool result = false;
  if (m_inf) {
    // When to check for EOF?

    while (m_inf && !result) {
      unsigned int start_line(m_linenum);
      // If we can read the molecule name, assume we'll find a
      // complete molecule.
      if (getline(name)) {
        getline(metadata); // User's info, program name etc.
        getline(comments);

        unsigned int num_atoms = 0;
        unsigned int num_bonds = 0;
        get_counts(num_atoms, num_bonds, counts_line);

        // A molecule could be just an atom, I suppose, but it can't
        // be just bonds.
        if (num_atoms > 0) {
          // If unable to read this molecule, skip to the next one.
          result =
              (read_atoms(atoms, num_atoms) && read_bonds(bonds, num_bonds) &&
               read_properties_block(properties_block) && read_tags(tags));
        }
        if (!result) {
          cerr << "Could not read molecule starting at line " << start_line
               << "; skipping ahead." << endl;
          skip_to_end();
        }
      }
    }
  }
  if (result) {
    next = Mol({.atoms = atoms,
                .bonds = bonds,
                .tags = tags,
                .name = name,
                .metadata = metadata,
                .comments = comments,
                .counts_line = counts_line,
                .properties_block = properties_block});
  } else {
    next = Mol();
  }
  return result;
}
} // namespace mesaac::mol
