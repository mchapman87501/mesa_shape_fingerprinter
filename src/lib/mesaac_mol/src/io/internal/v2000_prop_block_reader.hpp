#pragma once

#include "line_reader.hpp"
#include "mesaac_mol/atom.hpp"
#include "mesaac_mol/atom_props.hpp"
#include "mesaac_mol/bond.hpp"
#include "mesaac_mol/result.hpp"

namespace mesaac::mol::internal {

struct V2000PropBlockReader {
  // Goal: parse V2000 property blocks, updating atoms and bonds.
  using Result = mesaac::mol::Result<bool>;

  V2000PropBlockReader(LineReader &lines, AtomVector &atoms, BondVector &bonds)
      : m_lines(lines), m_atoms(atoms), m_bonds(bonds) {}

  Result read();

private:
  LineReader &m_lines;
  AtomVector &m_atoms;
  BondVector &m_bonds;
};

} // namespace mesaac::mol::internal
