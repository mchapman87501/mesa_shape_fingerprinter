
#include "v2000_prop_block_reader.hpp"

#include <format>
#include <functional>
#include <iostream>

#include "v2000_field_read_fns.hpp"

namespace mesaac::mol::internal {

namespace {
using Result = V2000PropBlockReader::Result;

using IndexedAtomIntValFn =
    std::function<void(AtomVector &, unsigned int, int)>;

Result process_atom_prop_line(const std::string &line, AtomVector &atoms,
                              IndexedAtomIntValFn &func) {
  /// an "M  CHG" or "M  RAD" line has content of the form
  /// nnnaaavvv[aaavvv]
  /// where "nnn" is the number of entries on the line,
  /// "aaa" is a one-based atom index,
  /// "vvv" is the charge for the indexed atom.

  /// TODO:
  /// Any atoms not listed in the line, or in a RAD line, must have their
  /// charges reset to zero.

  unsigned int num_entries;
  if (!uint_field(line, 6, 3, num_entries)) {
    return Result::Err(
        std::format("Could not read number of entries from '{}'.", line));
  }
  unsigned int i_start = 9;
  const unsigned int d_i = 4;
  for (unsigned int i_entry = 0; i_entry != num_entries; ++i_entry) {
    unsigned int atom_index;
    int value;
    if (!uint_field(line, i_start, d_i, atom_index)) {
      return Result::Err(
          std::format("Could not read atom index {} at column {} of \n'{}'",
                      i_entry + 1, i_start, line));
    }
    if (atom_index < 1) {
      return Result::Err("Atom index must be greater than zero.");
    }

    i_start += d_i;
    if (!int_field(line, i_start, d_i, value)) {
      return Result::Err(
          std::format("Could not read value {} at column {} of \n'{}'",
                      i_entry + 1, i_start, line));
    }
    i_start += d_i;

    func(atoms, atom_index, value);
  }
  return Result::Ok(true);
}

Result process_charge_line(const std::string &line, AtomVector &atoms) {
  IndexedAtomIntValFn update_chg = [](AtomVector &atoms, unsigned int index,
                                      int value) {
    atoms.at(index - 1).mutable_props().chg = value;
  };
  return process_atom_prop_line(line, atoms, update_chg);
}

Result process_radical_line(const std::string &line, AtomVector &atoms) {
  IndexedAtomIntValFn update_rad = [](AtomVector &atoms, unsigned int index,
                                      int value) {
    atoms.at(index - 1).mutable_props().rad = value;
  };
  return process_atom_prop_line(line, atoms, update_rad);
}

// The V2000 spec is unclear regarding the effect of M  CHG and M  RAD
// lines.  Specifically, it's unclear whether "M  CHG" should reset both
// the charge /and radical/ properties of every unlisted atom.
bool reset_charges_and_rads(AtomVector &atoms, bool already_reset) {
  if (!already_reset) {
    for (auto &atom : atoms) {
      atom.mutable_props().chg = 0;
      atom.mutable_props().rad = 0;
    }
  }
  return true;
}

} // namespace

V2000PropBlockReader::Result V2000PropBlockReader::read() {
  const auto prefix = "M  ";
  bool has_reset_charges = false;
  for (;;) {
    const auto read_result = m_lines.next();
    if (!read_result.is_ok()) {
      // Presume end of file.
      break;
    }
    const auto line = read_result.value();
    if (!line.starts_with(prefix)) {
      // Unsupported: "not used in current products" lines A, and G.
      // Unsupported: V (atom value) lines
      // Unsupported: Link Atom and all other "[Query]" lines
      // Unsupported and problematic: S lines.  If a skip line is encountered,
      // this code will fail to skip the indicated lines.

      // How to capture warnings, while continuing?
      std::cerr << m_lines.message(std::format(
                       "WARNING: Unsupported V2000 CTAB property line '{}'",
                       line))
                << std::endl;
    } else {
      if (line == "M  END") {
        break;
      }

      if (line.starts_with("M  CHG")) {
        has_reset_charges = reset_charges_and_rads(m_atoms, has_reset_charges);
        const auto result = process_charge_line(line, m_atoms);
        if (!result.is_ok()) {
          return Result::Err(m_lines.message(result.error()));
        }

      } else if (line.starts_with("M  RAD")) {
        has_reset_charges = reset_charges_and_rads(m_atoms, has_reset_charges);
        const auto result = process_radical_line(line, m_atoms);
        if (!result.is_ok()) {
          return Result::Err(m_lines.message(result.error()));
        }
      } // TODO support ISO (isotope) lines
      else {
        std::cerr << m_lines.message(std::format(
                         "WARNING: Ignoring property line '{}'", line))
                  << std::endl;
      }
    }
  }
  return Result::Ok(true);
}
} // namespace mesaac::mol::internal
