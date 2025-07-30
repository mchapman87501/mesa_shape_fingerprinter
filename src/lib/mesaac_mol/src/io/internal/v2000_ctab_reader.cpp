//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include "v2000_ctab_reader.hpp"

#include "mesaac_mol/element_info.hpp"

namespace mesaac::mol::internal {

namespace {
bool float_field(const std::string &line, unsigned int i_start,
                 unsigned int i_len, float &value) {
  try {
    value = std::stof(line.substr(i_start, i_len));
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

bool uint_field(const std::string &line, unsigned int i_start,
                unsigned int i_len, unsigned int &value) {
  try {
    value = std::stoul(line.substr(i_start, i_len));
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

} // namespace

bool V2000CTabReader::read(const MolHeaderBlock &header_block, CTab &ctab) {
  unsigned int num_atoms = 0;
  unsigned int num_bonds = 0;
  get_counts(header_block.counts_line(), num_atoms, num_bonds);
  // A molecule could be just an atom, but it can't
  // be just bonds.
  if (num_atoms > 0) {
    AtomVector atoms;
    BondVector bonds;
    std::string raw_properties_block;
    if (read_atoms(atoms, num_atoms) && read_bonds(bonds, num_bonds) &&
        read_properties_block(raw_properties_block)) {
      ctab = CTab{.name = header_block.name(),
                  .metadata = header_block.metadata(),
                  .comments = header_block.comments(),
                  .counts_line = header_block.counts_line(),
                  .atoms = atoms,
                  .bonds = bonds,
                  .raw_properties_block = raw_properties_block,
                  .post_ctab_block = ""};
      return true;
    }
  }
  return false;
}

void V2000CTabReader::get_counts(const std::string &line,
                                 unsigned int &num_atoms,
                                 unsigned int &num_bonds) {
  num_atoms = 0;
  num_bonds = 0;
  if (line.size() < 38) {
    std::cerr << m_lines.file_pos()
              << "WARNING: SD counts line has incorrect length" << std::endl
              << "expected at least 38 characters, got " << line.size()
              << std::endl
              << "from line '" << line << "'" << std::endl;
  } else {
    if (!uint_field(line, 0, 3, num_atoms)) {
      std::cerr << m_lines.file_pos() << "Could not read number of atoms "
                << "from '" << line << "'" << std::endl;
    }
    if (!uint_field(line, 3, 6, num_bonds)) {
      std::cerr << m_lines.file_pos() << "Could not read number of bonds "
                << "from '" << line << "'" << std::endl;
    }
  }
}

bool V2000CTabReader::read_atoms(AtomVector &atoms, unsigned int num_atoms) {
  atoms.reserve(num_atoms);
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

bool V2000CTabReader::read_bonds(BondVector &bonds, unsigned int num_bonds) {
  bonds.reserve(num_bonds);
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

bool V2000CTabReader::read_properties_block(std::string &properties_block) {
  bool result = true;
  std::ostringstream blockf;
  std::string line;
  while (m_lines.next(line)) {
    // TODO check for valid property name
    if (!line.starts_with("M  ")) {
      // TODO - exceptions
      std::cerr << m_lines.file_pos()
                << "WARNING: Invalid V2000 CTAB property line '" << line << "'"
                << std::endl;
      result = false;
    }
    blockf << line << std::endl;
    if (line == "M  END") {
      break;
    }
  }
  properties_block = blockf.str();

  return result;
}

std::optional<Atom> V2000CTabReader::read_next_atom() {
  std::string line;
  if (!m_lines.next(line)) {
    return std::nullopt;
  }
  // TODO: Enough w. the inline literal constants.
  if (line.size() < 34) {
    std::cerr << m_lines.file_pos() << "Atom line is too short: '" << line
              << "'." << std::endl;
    return std::nullopt;
  }

  float x, y, z;
  if (float_field(line, 0, 10, x) && float_field(line, 10, 10, y) &&
      float_field(line, 20, 10, z)) {
    return Atom({.atomic_num = get_atomic_num(line.substr(31, 3)),
                 .pos = {x, y, z},
                 .optional_cols = line.substr(34)});
  }
  std::cerr << m_lines.file_pos() << "Could not parse coordinates from '"
            << line << "'." << std::endl;
  return std::nullopt;
}

std::optional<Bond> V2000CTabReader::read_next_bond() {
  std::string line;
  if (!m_lines.next(line)) {
    return std::nullopt;
  }
  if (line.size() < 12) {
    std::cerr << m_lines.file_pos() << "Warning: Bond line is too short ("
              << line.size() << " characters): '" << line << "'." << std::endl;
  }

  // Try to read anyway:
  unsigned int a0, a1;
  BondType bond_type;
  unsigned int uint_bond_type;
  BondStereo stereo;
  unsigned int uint_stereo;
  if (uint_field(line, 0, 3, a0) && uint_field(line, 3, 6, a1) &&
      // So much for enum-driven value safety:
      uint_field(line, 6, 9, uint_bond_type) &&
      uint_field(line, 9, 12, uint_stereo)) {
    std::string optional_cols(line.substr(12));
    bond_type = static_cast<BondType>(uint_bond_type);
    stereo = static_cast<BondStereo>(uint_stereo);
    return Bond({.a0 = a0,
                 .a1 = a1,
                 .bond_type = bond_type,
                 .stereo = stereo,
                 .optional_cols = optional_cols});
  }
  std::cerr << m_lines.file_pos() << "Could not parse bond from '" << line
            << "'." << std::endl;
  return std::nullopt;
}

} // namespace mesaac::mol::internal
