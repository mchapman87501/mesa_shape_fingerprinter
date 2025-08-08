#pragma once

#include <string>

#include "mesaac_mol/result.hpp"

#include <optional>

#include "ctab.hpp"
#include "line_reader.hpp"
#include "mol_header_block.hpp"
#include "sdtags_reader.hpp"

namespace mesaac::mol::internal {

// Reads V2000 Mol blocks from an SD file.
struct V2000CTabReader {
  using Result = mesaac::mol::Result<CTab>;

  V2000CTabReader(LineReader &lines) : m_lines(lines), m_tags(lines) {}

  Result read(const MolHeaderBlock &header_block);

private:
  [[nodiscard]] mesaac::mol::Result<std::pair<unsigned int, unsigned int>>
  get_counts(const std::string &line);

  [[nodiscard]] mesaac::mol::Result<AtomVector>
  read_atoms(unsigned int num_atoms);
  [[nodiscard]] mesaac::mol::Result<BondVector>
  read_bonds(unsigned int num_bonds);
  [[nodiscard]] mesaac::mol::Result<std::string>
  read_properties_block(AtomVector &atoms, BondVector &bonds);
  [[nodiscard]] mesaac::mol::Result<Atom>
  read_atom(const unsigned int atom_index);
  [[nodiscard]] mesaac::mol::Result<Bond> read_next_bond();

private:
  LineReader &m_lines;
  SDTagsReader m_tags;
};

} // namespace mesaac::mol::internal
