#pragma once
//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include <optional>

#include "ctab.hpp"
#include "line_reader.hpp"
#include "mol_header_block.hpp"
#include "sdtags_reader.hpp"

namespace mesaac::mol::internal {

// Reads V2000 Mol blocks from an SD file.
struct V2000CTabReader {
  V2000CTabReader(LineReader &lines) : m_lines(lines), m_tags(lines) {}

  bool read(const MolHeaderBlock &header_block, CTab &ctab);

private:
  void get_counts(const std::string &line, unsigned int &num_atoms,
                  unsigned int &num_bonds);

  [[nodiscard]] bool read_atoms(AtomVector &atoms, unsigned int num_atoms);
  [[nodiscard]] bool read_bonds(BondVector &bonds, unsigned int num_bonds);
  [[nodiscard]] bool read_properties_block(std::string &properties_block);
  [[nodiscard]] std::optional<Atom> read_next_atom();
  [[nodiscard]] std::optional<Bond> read_next_bond();

private:
  LineReader &m_lines;
  SDTagsReader m_tags;
};

} // namespace mesaac::mol::internal
