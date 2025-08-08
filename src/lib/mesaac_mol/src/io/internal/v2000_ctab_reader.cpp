//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include "v2000_ctab_reader.hpp"

#include <format>

#include "mesaac_mol/atom_props.hpp"
#include "mesaac_mol/element_info.hpp"

#include "v2000_field_read_fns.hpp"
#include "v2000_prop_block_reader.hpp"

namespace mesaac::mol::internal {

namespace {
using CountResult = Result<std::pair<unsigned int, unsigned int>>;
using AtomResult = mesaac::mol::Result<Atom>;
using AtomsResult = mesaac::mol::Result<AtomVector>;

using BondResult = mesaac::mol::Result<Bond>;
using BondsResult = mesaac::mol::Result<BondVector>;

using StrResult = mesaac::mol::Result<std::string>;

} // namespace

V2000CTabReader::Result
V2000CTabReader::read(const MolHeaderBlock &header_block) {
  const auto count_result = get_counts(header_block.counts_line());
  if (!count_result.is_ok()) {
    return V2000CTabReader::Result::Err(count_result.error());
  }
  const auto [num_atoms, num_bonds] = count_result.value();
  // A molecule could be just an atom, but it can't
  // be just bonds.
  const auto atoms_result = read_atoms(num_atoms);
  if (!atoms_result.is_ok()) {
    return V2000CTabReader::Result::Err(atoms_result.error());
  }
  const auto bonds_result = read_bonds(num_bonds);
  if (!bonds_result.is_ok()) {
    return V2000CTabReader::Result::Err(bonds_result.error());
  }

  auto atoms = atoms_result.value();
  auto bonds = bonds_result.value();
  const auto props_result = read_properties_block(atoms, bonds);

  if (!props_result.is_ok()) {
    return V2000CTabReader::Result::Err(props_result.error());
  }
  return V2000CTabReader::Result::Ok(
      CTab{.name = header_block.name(),
           .metadata = header_block.metadata(),
           .comments = header_block.comments(),
           .counts_line = header_block.counts_line(),
           .atoms = atoms,
           .bonds = bonds,
           .raw_properties_block = props_result.value(),
           .post_ctab_block = ""});
}

CountResult V2000CTabReader::get_counts(const std::string &line) {
  unsigned int num_atoms = 0;
  unsigned int num_bonds = 0;
  if (line.size() < 38) {
    return CountResult::Err(m_lines.message(
        "Counts line is too short - expected at least 38 characters"));
  }
  if (!uint_field(line, 0, 3, num_atoms)) {
    return CountResult::Err(m_lines.message(
        std::format("Could not read number of atoms from '{}'", line)));
  }
  if (!uint_field(line, 3, 6, num_bonds)) {
    return CountResult::Err(m_lines.message(
        std::format("Could not read number of atoms from '{}'", line)));
  }
  return CountResult::Ok(std::make_pair(num_atoms, num_bonds));
}

AtomsResult V2000CTabReader::read_atoms(unsigned int num_atoms) {
  AtomVector atoms;
  atoms.reserve(num_atoms);
  for (unsigned int i = 0; i != num_atoms; i++) {
    const auto atom = read_atom(i);
    if (atom.is_ok()) {
      atoms.emplace_back(atom.value());
    } else {
      // When an error is encountered, give up on the current
      // molecule.
      return AtomsResult::Err(std::format("Could not read atom {}", i));
    }
  }
  return AtomsResult::Ok(atoms);
}

BondsResult V2000CTabReader::read_bonds(unsigned int num_bonds) {
  BondVector bonds;
  bonds.reserve(num_bonds);
  for (unsigned int i = 0; i != num_bonds; i++) {
    const auto bond = read_next_bond();
    if (bond.is_ok()) {
      bonds.emplace_back(bond.value());
    } else {
      return BondsResult::Err(std::format("Could not read bond {}", i));
    }
  }
  return BondsResult::Ok(bonds);
}

StrResult V2000CTabReader::read_properties_block(AtomVector &atoms,
                                                 BondVector &bonds) {
  V2000PropBlockReader block_reader(m_lines, atoms, bonds);
  const auto block_result = block_reader.read();
  if (!block_result.is_ok()) {
    return StrResult::Err(block_result.error());
  }
  return StrResult::Ok("");
}

AtomResult V2000CTabReader::read_atom(const unsigned int atom_index) {
  const auto read_result = m_lines.next();
  if (!read_result.is_ok()) {
    return AtomResult::Err(read_result.error());
  }
  const auto line = read_result.value();
  // TODO: Enough w. the inline literal constants.
  if (line.size() < 34) {
    return AtomResult::Err(
        m_lines.message(std::format("Atom line is too short: '{}'.", line)));
  }

  float x, y, z;
  if (!(float_field(line, 0, 10, x) && float_field(line, 10, 10, y) &&
        float_field(line, 20, 10, z))) {
    return AtomResult::Err(m_lines.message(
        std::format(" Could not extract atom coords from '{}'", line)));
  }

  // It is not clear from the V2000 spec which of the following fields are
  // required, and which are optional.  This code, using cox2_3d.sdf as a
  // guide -
  // xxxxx.xxxxyyyyy.yyyyzzzzz.zzzz aaaddcccssshhhbbbvvvHHHrrriiimmmnnneee
  //    27.7051   22.0403   17.0243 C   0  0  0  0  0  0
  // assumes everything after the atomic symbol is optional, with a default
  // value of zero:
  // mass difference, charge, atom stereo parity, hydrogen count, stereo care
  // box, valence, H0 designator, atom-atom-mapping number, inversion/retention
  // flag, exact change flag.

  // An example of a /complete/ atom line is from the herg actives:
  //    -4.3689   -2.9358   -5.6761 C   0  0  0  0  0  0  0  0  0  0  0  0
  // Field start columns, zero-based are:
  // x:0, y:10, z:20, a:31, d:34, c:36, s:39, h:42, b:45, v:48, H:51, r:54,
  // i:57, m:60, n:63, e:66

  const std::string atomic_symbol(line.substr(31, 3));

  unsigned char atomic_num = 0;
  try {
    atomic_num = get_atomic_num(atomic_symbol);
  } catch (std::invalid_argument &e) {
    return AtomResult::Err(m_lines.message(e.what()));
  }

  float atomic_mass;
  try {
    atomic_mass = get_atomic_mass(atomic_num);
  } catch (std::invalid_argument &e) {
    return AtomResult::Err(m_lines.message(e.what()));
  }

  const int mass_diff = optional_int_field(line, 34, 2);
  // Use a default mass of zero, if there is no mass diff.
  // Would it be better to use the element info's mass in such a case?
  const float mass = (mass_diff == 0) ? 0.0 : atomic_mass + mass_diff;

  const int charge = optional_int_field(line, 36, 3);
  const unsigned int stereo_parity = optional_uint_field(line, 39, 3);
  const int hydrogen_count = optional_int_field(line, 42, 3);
  const unsigned int stereo_care_box = optional_uint_field(line, 45, 3);
  const int valence = optional_int_field(line, 48, 3);
  const unsigned int atom_atom_mapping = optional_uint_field(line, 60, 3);
  const unsigned int inversion_retention_flag =
      optional_uint_field(line, 63, 3);
  const unsigned int exact_change_flag = optional_uint_field(line, 66, 3);

  const AtomProps props{
      .index = atom_index,
      .aamap = atom_atom_mapping,
      .chg = charge,
      .rad = 0,
      .cfg = stereo_parity,
      .mass = mass,
      .val = valence,
      // Need to double-check this -- I *think* I can ignore h0_designator.
      // See Mick Kappler's presentation at CUP 5:
      // https://www.daylight.com/meetings/mug05/Kappler/ctfile.pdf
      // "[ISIS/Desktop] Redundant with hydrogen count information.  May be
      // unsupported in future releases of MDL software."
      .hcount = hydrogen_count,
      .stbox = stereo_care_box,
      .invret = inversion_retention_flag,
      .exachg = exact_change_flag,
  };

  return AtomResult::Ok(Atom({.atomic_num = atomic_num,
                              .pos = {x, y, z},
                              .props = props,
                              .optional_cols = line.substr(34)}));
}

BondResult V2000CTabReader::read_next_bond() {
  const auto read_result = m_lines.next();
  if (!read_result.is_ok()) {
    return BondResult::Err(read_result.error());
  }
  const auto line = read_result.value();

  if (line.size() < 12) {
    std::cerr << m_lines.message(std::format(
                     "Warning: Bond line is too short ({} characters): '{}'.",
                     line.size(), line))
              << std::endl;
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
    return BondResult::Ok(Bond({.a0 = a0,
                                .a1 = a1,
                                .bond_type = bond_type,
                                .stereo = stereo,
                                .optional_cols = optional_cols}));
  }
  return BondResult::Err(
      m_lines.message(std::format("Could not parse bond from '{}'.", line)));
}

} // namespace mesaac::mol::internal
