//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include "v3000_ctab_reader.hpp"
#include "mesaac_mol/element_info.hpp"
namespace mesaac::mol::internal {

namespace {
static const std::string v3000_prefix = "M  V30 ";

// Read a V3000 atom type from a stream
bool read_atom_type(std::istream &ins, std::string &atom_type) {
  // Skip whitespace
  ins >> std::ws;
  char ch = ins.get();
  if (ch == '"') {
    // atom_type is a double-quote-delimited string.  I believe escaped/embedded
    // double quotes are not supported.
    if (std::getline(ins, atom_type, '"')) {
      return true;
    }
  } else {
    // atom_type is a string containing no spaces.
    atom_type = std::string(1, ch);
    while (ins.get(ch)) {
      if (std::isspace(ch)) {
        return true;
      }
      atom_type += ch;
    }
  }
  return false;
}

bool unsupported_atom_type(const std::string &atom_type) {
  return (atom_type == "R#" || atom_type == "*" || atom_type.starts_with("[") ||
          atom_type.find(" ") != std::string::npos);
}
} // namespace

bool V3000CTabReader::read(const MolHeaderBlock &header_block, CTab &ctab) {
  std::string line;
  if (!check_v30_line_eq("BEGIN CTAB")) {
    std::cerr << m_lines.file_pos() << "Did not find BEGIN CTAB line."
              << std::endl;
    return false;
  }

  if (!next_v30_line(line)) {
    std::cerr << m_lines.file_pos() << "Could not read V30 line.  Got '" << line
              << "'." << std::endl;
  } else {
    size_t num_atoms;
    size_t num_bonds;
    if (!get_counts(line, num_atoms, num_bonds)) {
      std::cerr << "Could not get counts from '" << line << "'." << std::endl;
    } else {
      AtomVector atoms;
      BondVector bonds;
      std::string raw_other_properties;
      std::string raw_rgroups_etc;
      if (read_atoms(atoms, num_atoms) && read_bonds(bonds, num_bonds) &&
          read_other_blocks(raw_other_properties, "M  V30 END CTAB") &&
          read_other_blocks(raw_rgroups_etc, "M  END"))

      {
        // XXX FIX THIS - neither blocks within the main CTAB nor
        // subsequent block such as Rgroups are preserved for later
        // writing.
        ctab = CTab{.name = header_block.name(),
                    .metadata = header_block.metadata(),
                    .comments = header_block.comments(),
                    .counts_line = header_block.counts_line(),
                    .atoms = atoms,
                    .bonds = bonds,
                    .raw_properties_block = raw_other_properties,
                    .post_ctab_block = raw_rgroups_etc};
        return true;
      }
    }
  }
  return false;
}

bool V3000CTabReader::get_counts(const std::string &counts_line,
                                 size_t &num_atoms, size_t &num_bonds) {
  const std::string counts_prefix = "COUNTS";

  if (!counts_line.starts_with(counts_prefix)) {
    return print_parse_error(counts_prefix, counts_line);
  }
  const std::string tail = counts_line.substr(counts_prefix.size());

  std::istringstream ins(tail);
  if (!(ins >> num_atoms >> num_bonds)) {
    return print_parse_error("ATOM_COUNT' 'BOND_COUNT", tail);
  }
  return true;
}

bool V3000CTabReader::read_atoms(AtomVector &atoms, unsigned int num_atoms) {
  atoms.reserve(num_atoms);
  std::string line;
  if (!check_v30_line_eq("BEGIN ATOM")) {
    return false;
  }
  for (unsigned int i = 0; i != num_atoms; i++) {
    const auto atom = read_next_atom();
    if (atom.has_value()) {
      atoms.emplace_back(atom.value());
    } else {
      // When an error is encountered, give up on the current
      // molecule.
      std::cerr << "Failed to read atom " << i << std::endl;
      return false;
    }
  }
  return check_v30_line_eq("END ATOM");
}

std::optional<Atom> V3000CTabReader::read_next_atom() {
  std::string line;

  if (next_v30_line(line)) {
    // There are a lot of V3000 atom attributes that are not relevant
    // for mesaac_mol, which is focused mainly on geometry /volume of molecules.
    // TODO record these attributes so they can be preserved when writing
    // to SD files.
    unsigned int atom_index = 0;

    m_atom_ins.clear();
    m_atom_ins.str(line);

    if (!(m_atom_ins >> atom_index)) {
      std::cerr << m_lines.file_pos() << "Could not read atom index from '"
                << line << "'." << std::endl;
      return std::nullopt;
    }

    std::string atom_type;
    if (!read_atom_type(m_atom_ins, atom_type)) {
      std::cerr << m_lines.file_pos() << "Could not read atom type."
                << std::endl;
      return std::nullopt;
    }
    if (unsupported_atom_type(atom_type)) {
      std::cerr << "Query atoms are not supported: '" << atom_type << "'"
                << std::endl;
      return std::nullopt;
    }

    float x, y, z;
    if (!(m_atom_ins >> x >> y >> z)) {
      std::cerr << "Could not read atom coordinates." << std::endl;
      return std::nullopt;
    }

    return Atom({.atomic_num = get_atomic_num(atom_type),
                 .pos = {x, y, z},
                 .optional_cols = m_atom_ins.str()});
  }
  return std::nullopt;
}

bool V3000CTabReader::read_bonds(BondVector &bonds, unsigned int num_bonds) {
  if (num_bonds == 0) {
    return true;
  }
  bonds.reserve(num_bonds);
  std::string line;

  if (!check_v30_line_eq("BEGIN BOND")) {
    return false;
  }

  for (unsigned int i = 0; i != num_bonds; ++i) {
    const auto bond = read_next_bond();
    if (bond.has_value()) {
      bonds.emplace_back(bond.value());
    } else {
      return false;
    }
  }

  return check_v30_line_eq("END BOND");
}

std::optional<Bond> V3000CTabReader::read_next_bond() {
  std::string line;

  if (next_v30_line(line)) {
    m_atom_ins.clear();
    m_atom_ins.str(line);
    // XXX FIX THIS As for atoms, so to does a bond have extra properties
    // such as index that are not captured here.
    unsigned int bond_index = 0, i_bond_type = 0, a0 = 0, a1 = 0;
    if (m_atom_ins >> bond_index >> i_bond_type >> a0 >> a1) {
      // XXX FIX THIS
      // The bond stereo value is read from an optional CFG=n field.
      // This code ignores that value, which is going to screw up
      // preservation of properties on output....
      std::string remainder;
      std::getline(m_atom_ins, remainder, '\0');
      constexpr auto stereo = BondStereo::bs_not_stereo;
      return Bond({.a0 = a0,
                   .a1 = a1,
                   .bond_type = static_cast<BondType>(i_bond_type),
                   .stereo = stereo,
                   .optional_cols = remainder});
    }
    std::cerr << "Failed to read bond.  Properties as read: " << bond_index
              << " " << i_bond_type << " " << a0 << " " << a1 << std::endl;
  }
  return std::nullopt;
}

// Read and ignore various block types.  These should be preserved and
// written when the Mol is written.
bool V3000CTabReader::read_other_blocks(std::string &other_blocks,
                                        const std::string &terminator) {
  bool result = true;
  std::ostringstream blockf;
  std::string line;

  while (m_lines.next(line)) {
    blockf << line << std::endl;
    if (line == terminator) {
      break;
    }
    if (!line.starts_with(v3000_prefix)) {
      // TODO - exceptions
      std::cerr << m_lines.file_pos()
                << "WARNING: Invalid V3000 CTAB property line '" << line << "'"
                << std::endl;
      result = false;
    }
  }
  other_blocks = blockf.str();

  return result;
}

bool V3000CTabReader::strip_prefix(const std::string &line,
                                   std::string &stripped) const {
  bool result = line.starts_with(v3000_prefix);
  stripped = result ? line.substr(v3000_prefix.size()) : line;
  if (!result) {
    std::cerr << m_lines.file_pos() << "Missing expected prefix '"
              << v3000_prefix << "'" << std::endl;
  }
  return result;
}

// Read the next line, expecting a "M  V30" line and stripping the
// prefix. Combine continuation lines.
// @return true if result was successfully read.
bool V3000CTabReader::next_v30_line(std::string &result) const {
  std::string line, stripped;
  if (m_lines.next(line) && strip_prefix(line, stripped)) {
    if (stripped.ends_with("-")) {
      // It's a continuation line.
      std::string cont;
      if (m_lines.next(line) && strip_prefix(line, cont)) {
        result = stripped.substr(0, stripped.size() - 1) + cont;
        return true;
      }
      return false;
    }
    result = stripped;
    return true;
  }
  return false;
}

bool V3000CTabReader::check_v30_line_eq(const std::string &expected) {
  std::string line;
  bool got_expected = next_v30_line(line) && (line == expected);
  if (!got_expected) {
    (void)print_parse_error(expected, line);
  }
  return got_expected;
}

bool V3000CTabReader::print_parse_error(const std::string &expected,
                                        const std::string &actual) {
  std::cerr << m_lines.file_pos() << "Parse error.  Expected '" << expected
            << "', got '" << actual << "'." << std::endl;
  return false;
}

} // namespace mesaac::mol::internal
