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
  V3000CTabReader(LineReader &lines) : m_lines(lines), m_tags(lines) {}
  [[nodiscard]] bool read(const MolHeaderBlock &header_block, CTab &ctab);

private:
  [[nodiscard]] bool get_counts(const std::string &counts_line,
                                size_t &num_atoms, size_t &num_bonds);
  [[nodiscard]] bool read_atoms(AtomVector &atoms, unsigned int num_atoms);
  [[nodiscard]] bool read_bonds(BondVector &bonds, unsigned int num_bonds);

  [[nodiscard]] std::optional<Atom> read_next_atom();
  [[nodiscard]] std::optional<Bond> read_next_bond();

  [[nodiscard]] bool read_other_blocks(std::string &other_blocks,
                                       const std::string &terminator);

  [[nodiscard]] bool strip_prefix(const std::string &line,
                                  std::string &stripped) const;
  [[nodiscard]] bool next_v30_line(std::string &result) const;
  [[nodiscard]] bool check_v30_line_eq(const std::string &expected);
  [[nodiscard]] bool print_parse_error(const std::string &expected,
                                       const std::string &actual);

private:
  LineReader &m_lines;
  SDTagsReader m_tags;
  std::istringstream m_atom_ins;
};

} // namespace mesaac::mol::internal
