#pragma once
//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include <optional>
#include <sstream>

#include "ctab.hpp"
#include "line_reader.hpp"
#include "mol_header_block.hpp"
#include "sdtags_reader.hpp"

namespace mesaac::mol::internal {

// Reads V3000 Mol blocks from an SD file.
struct V3000CTabReader {
  using Result = mesaac::mol::Result<CTab>;

  V3000CTabReader(LineReader &lines) : m_lines(lines), m_tags(lines) {}
  [[nodiscard]] Result read(const MolHeaderBlock &header_block);

private:
  using StrResult = mesaac::mol::Result<std::string>;

  [[nodiscard]] StrResult get_counts(const std::string &counts_line,
                                     size_t &num_atoms, size_t &num_bonds);
  [[nodiscard]] mesaac::mol::Result<AtomVector>
  read_atoms(unsigned int num_atoms);
  [[nodiscard]] mesaac::mol::Result<BondVector>
  read_bonds(unsigned int num_bonds);

  [[nodiscard]] mesaac::mol::Result<Atom> read_next_atom();
  [[nodiscard]] mesaac::mol::Result<Bond> read_next_bond();

  [[nodiscard]] StrResult read_other_blocks(const std::string &terminator);

  [[nodiscard]] StrResult strip_prefix(const std::string &line) const;
  [[nodiscard]] StrResult next_v30_line() const;
  [[nodiscard]] StrResult check_v30_line_eq(const std::string &expected);
  [[nodiscard]] StrResult parse_error(const std::string &expected,
                                      const std::string &actual);

private:
  LineReader &m_lines;
  SDTagsReader m_tags;
  std::istringstream m_atom_bond_ins;
};

} // namespace mesaac::mol::internal
