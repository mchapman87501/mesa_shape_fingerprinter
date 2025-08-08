//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include "v3000_ctab_reader.hpp"

#include <format>

#include "mesaac_mol/element_info.hpp"
#include "v3000_atom_prop_reader.hpp"

namespace mesaac::mol::internal {

namespace {
using BoolResult = mesaac::mol::Result<bool>;
using AtomResult = mesaac::mol::Result<Atom>;
using AtomsResult = mesaac::mol::Result<AtomVector>;

using BondResult = mesaac::mol::Result<Bond>;
using BondsResult = mesaac::mol::Result<BondVector>;

using StrResult = mesaac::mol::Result<std::string>;

static constexpr std::string v3000_prefix = "M  V30 ";

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

V3000CTabReader::Result
V3000CTabReader::read(const MolHeaderBlock &header_block) {
  const auto begin_ctab = check_v30_line_eq("BEGIN CTAB");
  if (!begin_ctab.is_ok()) {
    return V3000CTabReader::Result::Err(begin_ctab.error());
  }

  const auto next_line = next_v30_line();
  if (!next_line.is_ok()) {
    return V3000CTabReader::Result::Err(next_line.error());
  }

  size_t num_atoms;
  size_t num_bonds;
  const auto counts_result =
      get_counts(next_line.value(), num_atoms, num_bonds);
  if (!counts_result.is_ok()) {
    return V3000CTabReader::Result::Err(counts_result.error());
  }

  const auto atoms_result = read_atoms(num_atoms);
  if (!atoms_result.is_ok()) {
    return V3000CTabReader::Result::Err(atoms_result.error());
  }
  const auto bonds_result = read_bonds(num_bonds);
  if (!bonds_result.is_ok()) {
    return V3000CTabReader::Result::Err(bonds_result.error());
  }
  const auto props_result = read_other_blocks("M  V30 END CTAB");
  if (!props_result.is_ok()) {
    return V3000CTabReader::Result::Err(props_result.error());
  }
  const auto rgroups_result = read_other_blocks("M  END");
  if (!rgroups_result.is_ok()) {
    return V3000CTabReader::Result::Err(rgroups_result.error());
  }

  // XXX FIX THIS - neither blocks within the main CTAB nor
  // subsequent block such as Rgroups are preserved for later
  // writing.
  return V3000CTabReader::Result::Ok(
      CTab{.name = header_block.name(),
           .metadata = header_block.metadata(),
           .comments = header_block.comments(),
           .counts_line = header_block.counts_line(),
           .atoms = atoms_result.value(),
           .bonds = bonds_result.value(),
           .raw_properties_block = props_result.value(),
           .post_ctab_block = rgroups_result.value()});
}

StrResult V3000CTabReader::get_counts(const std::string &counts_line,
                                      size_t &num_atoms, size_t &num_bonds) {
  const std::string counts_prefix = "COUNTS";

  if (!counts_line.starts_with(counts_prefix)) {
    return parse_error(counts_prefix, counts_line);
  }
  const std::string tail = counts_line.substr(counts_prefix.size());

  std::istringstream ins(tail);
  // istream seems to do the wrong thing when parsing a negative value
  // into an unsigned int.  Hence this.
  int signed_num_atoms, signed_num_bonds;
  if (!(ins >> signed_num_atoms >> signed_num_bonds)) {
    return parse_error("ATOM_COUNT' 'BOND_COUNT", tail);
  }
  if (signed_num_atoms < 0) {
    return StrResult::Err(
        m_lines.message("Number of atoms must be non-negative."));
  }
  if (signed_num_bonds < 0) {
    return StrResult::Err(
        m_lines.message("Number of bonds must be non-negative."));
  }
  num_atoms = signed_num_atoms;
  num_bonds = signed_num_bonds;
  return StrResult::Ok("");
}

AtomsResult V3000CTabReader::read_atoms(unsigned int num_atoms) {

  const auto begin_atom = check_v30_line_eq("BEGIN ATOM");
  if (!begin_atom.is_ok()) {
    return AtomsResult::Err(begin_atom.error());
  }

  AtomVector atoms;
  atoms.reserve(num_atoms);
  for (unsigned int i = 0; i != num_atoms; i++) {
    const auto atom_result = read_next_atom();
    if (atom_result.is_ok()) {
      atoms.emplace_back(atom_result.value());
    } else {
      // When an error is encountered, give up on the current
      // molecule.
      return AtomsResult::Err(atom_result.error());
    }
  }
  const auto end_atom = check_v30_line_eq("END ATOM");
  if (!end_atom.is_ok()) {
    return AtomsResult::Err(end_atom.error());
  }
  return AtomsResult::Ok(atoms);
}

AtomResult V3000CTabReader::read_next_atom() {
  const auto v30_line = next_v30_line();
  if (!v30_line.is_ok()) {
    return AtomResult::Err(v30_line.error());
  }
  // There are a lot of V3000 atom attributes that are not relevant
  // for mesaac_mol, which is focused mainly on geometry /volume of molecules.
  // TODO record these attributes so they can be preserved when writing
  // to SD files.
  unsigned int atom_index = 0;
  std::string line = v30_line.value();
  m_atom_bond_ins.clear();
  m_atom_bond_ins.str(line);

  if (!(m_atom_bond_ins >> atom_index)) {
    return AtomResult::Err(m_lines.message(
        std::format("Could not read atom index from '{}'.", line)));
  }

  std::string atom_type;
  if (!read_atom_type(m_atom_bond_ins, atom_type)) {
    return AtomResult::Err(
        m_lines.message(std::format("Could not read atom type from {}", line)));
  }
  if (unsupported_atom_type(atom_type)) {
    return AtomResult::Err(m_lines.message(
        std::format("Query atoms are not supported: '{}'", atom_type)));
  }

  float x, y, z;
  if (!(m_atom_bond_ins >> x >> y >> z)) {
    return AtomResult::Err(m_lines.message(
        std::format("Could not read atom coordinates from '{}'.", line)));
  }

  int aamap;
  if (!(m_atom_bond_ins >> aamap) || (aamap < 0)) {
    return AtomResult::Err(m_lines.message(
        std::format("Invalid atom-atom mapping {} from '{}.", aamap, line)));
  }

  V3000AtomPropReader prop_reader;
  const auto prop_result = prop_reader.read(m_atom_bond_ins);
  if (!prop_result.is_ok()) {
    return AtomResult::Err(m_lines.message(prop_result.error()));
  }
  auto props = prop_result.value();
  props.index = atom_index;
  props.aamap = aamap;

  return AtomResult::Ok(Atom({.atomic_num = get_atomic_num(atom_type),
                              .pos = {x, y, z},
                              .props = std::move(props),
                              .optional_cols = m_atom_bond_ins.str()}));
}

BondsResult V3000CTabReader::read_bonds(unsigned int num_bonds) {
  BondVector bonds;
  bonds.reserve(num_bonds);
  if (num_bonds == 0) {
    return BondsResult::Ok(bonds);
  }

  std::string line;

  const auto begin_bonds = check_v30_line_eq("BEGIN BOND");
  if (!begin_bonds.is_ok()) {
    return BondsResult::Err(begin_bonds.error());
  }

  for (unsigned int i = 0; i != num_bonds; ++i) {
    const auto bond = read_next_bond();
    if (bond.is_ok()) {
      bonds.emplace_back(bond.value());
    } else {
      return BondsResult::Err(bond.error());
    }
  }
  const auto end_bond = check_v30_line_eq("END BOND");
  if (!end_bond.is_ok()) {
    return BondsResult::Err(end_bond.error());
  }
  return BondsResult::Ok(bonds);
}

BondResult V3000CTabReader::read_next_bond() {
  const auto v30_line = next_v30_line();
  if (!v30_line.is_ok()) {
    return BondResult::Err(v30_line.error());
  }

  const auto line = v30_line.value();
  m_atom_bond_ins.clear();
  m_atom_bond_ins.str(line);
  // XXX FIX THIS As for atoms, so to does a bond have extra properties
  // such as index that are not captured here.
  unsigned int bond_index = 0, i_bond_type = 0, a0 = 0, a1 = 0;
  if (m_atom_bond_ins >> bond_index >> i_bond_type >> a0 >> a1) {
    // XXX FIX THIS
    // The bond stereo value is read from an optional CFG=n field.
    // This code ignores that value, which is going to screw up
    // preservation of properties on output....
    std::string remainder;
    std::getline(m_atom_bond_ins, remainder, '\0');
    constexpr auto stereo = BondStereo::bs_not_stereo;
    return BondResult::Ok(Bond({.a0 = a0,
                                .a1 = a1,
                                .bond_type = static_cast<BondType>(i_bond_type),
                                .stereo = stereo,
                                .optional_cols = remainder}));
  }
  return BondResult::Err(
      m_lines.message(std::format("Failed to read bond from '{}'.", line)));
}

// Read and ignore various block types.  These should be preserved and
// written when the Mol is written.
StrResult V3000CTabReader::read_other_blocks(const std::string &terminator) {
  std::ostringstream blockf;

  for (;;) {
    const auto read_result = m_lines.next();
    if (!read_result.is_ok()) {
      return read_result;
    }

    const auto line = read_result.value();
    blockf << line << std::endl;

    if (line == terminator) {
      return StrResult::Ok(blockf.str());
    }

    if (!line.starts_with(v3000_prefix)) {
      // Log a warning, and keep going.
      std::cerr << m_lines.message(std::format(
                       "WARNING: Invalid V3000 CTAB property line '{}'.", line))
                << std::endl;
    }
  }
}

StrResult V3000CTabReader::strip_prefix(const std::string &line) const {
  bool result = line.starts_with(v3000_prefix);
  std::string stripped = result ? line.substr(v3000_prefix.size()) : line;
  if (!result) {
    return StrResult::Err(m_lines.message(
        std::format("Missing expected prefix '{}'.", v3000_prefix)));
  }
  return StrResult::Ok(stripped);
}

/**
 * @brief Read the next "M  V30 " line, stripping the prefix.  Combine
 * continuation lines.
 * @param result the line(s), sans prefix
 * @return true if result was successfully read
 */
StrResult V3000CTabReader::next_v30_line() const {
  std::string result = "";
  for (;;) {
    const auto read_result = m_lines.next();
    if (!read_result.is_ok()) {
      return StrResult::Err(read_result.error());
    }
    const auto strip_result = strip_prefix(read_result.value());
    if (strip_result.is_ok()) {
      const std::string stripped = strip_result.value();
      if (!stripped.ends_with("-")) {
        // It's not a continuation line, so hey, we're done.
        return StrResult::Ok(result + stripped);
      } else {
        // It's a continuation line.  Accumulate and go again.
        result += stripped.substr(0, stripped.size() - 1);
      }
    } else {
      // Return the stripping failure.
      return strip_result;
    }
  }
}

StrResult V3000CTabReader::check_v30_line_eq(const std::string &expected) {
  std::string line;
  const auto line_result = next_v30_line();
  bool got_expected = line_result.is_ok() && line_result.value() == expected;
  if (!got_expected) {
    return parse_error(expected, line);
  }
  return line_result;
}

StrResult V3000CTabReader::parse_error(const std::string &expected,
                                       const std::string &actual) {
  return StrResult::Err(m_lines.message(
      std::format("Parse error: Expected '{}', got '{}'", expected, actual)));
}

} // namespace mesaac::mol::internal
