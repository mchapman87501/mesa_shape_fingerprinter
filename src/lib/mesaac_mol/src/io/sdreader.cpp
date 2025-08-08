//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/io/sdreader.hpp"

#include <format>
#include <sstream>
#include <string>
#include <vector>

#include "mesaac_mol/element_info.hpp"

#include "internal/line_reader.hpp"
#include "internal/mol_header_block.hpp"
#include "internal/sdtags_reader.hpp"
#include "internal/v2000_ctab_reader.hpp"
#include "internal/v3000_ctab_reader.hpp"

using namespace std;

namespace mesaac::mol {

struct SDReaderImpl {
  SDReaderImpl(std::istream &inf, const std::string &description)
      : m_lines(inf, description), m_v2000(m_lines), m_v3000(m_lines) {}

  using CTabResult = mesaac::mol::Result<internal::CTab>;

  MolResult read() {
    const auto ctab_result = read_molfile();
    if (!ctab_result.is_ok()) {
      return MolResult::Err(ctab_result.error());
    }

    const auto ctab = ctab_result.value();
    const auto tags_result = read_tags();
    if (tags_result.is_ok()) {
      const auto tags = tags_result.value();
      return MolResult::Ok(Mol({
          .atoms = ctab.atoms,
          .bonds = ctab.bonds,
          .tags = tags,
          .name = ctab.name,
          .metadata = ctab.metadata,
          .comments = ctab.comments,
          .counts_line = ctab.counts_line,
          .properties_block = ctab.raw_properties_block,
      }));
    }
    return MolResult::Err(tags_result.error());
  }

  // Skip the next mol.
  BoolResult skip() {
    if (m_lines.eof()) {
      return BoolResult::Ok(true);
    }
    return skip_to_end();
  }

  bool eof() const { return m_lines.eof(); }

private:
  CTabResult read_molfile() {
    const auto header_result = internal::MolHeaderBlock::read(m_lines);
    if (!header_result.is_ok()) {
      if (m_lines.eof()) {
        return CTabResult::Err("End of file");
      }
      return CTabResult::Err(header_result.error());
    }
    const auto header = header_result.value();
    return header.is_v3000() ? m_v3000.read(header) : m_v2000.read(header);
  }

  internal::SDTagsReader::Result read_tags() {
    internal::SDTagsReader tags_reader(m_lines);
    return tags_reader.read();
  }

  // Skip to the end of the current mol.
  BoolResult skip_to_end() {
    while (!m_lines.eof()) {
      const auto result = m_lines.next();
      if (result.is_ok()) {
        if (result.value() == "$$$$") {
          return BoolResult::Ok(true);
        }
      } else {
        return BoolResult::Err(result.error());
      }
    }
    return BoolResult::Ok(true);
  }

  internal::LineReader m_lines;
  internal::V2000CTabReader m_v2000;
  internal::V3000CTabReader m_v3000;
};

// SDReader:
SDReader::SDReader(istream &inf, const string &pathname)
    : m_impl(std::make_unique<SDReaderImpl>(inf, pathname)) {}

SDReader::~SDReader() {}

BoolResult SDReader::skip() { return m_impl->skip(); }

MolResult SDReader::read() { return m_impl->read(); }

bool SDReader::eof() const { return m_impl->eof(); }

} // namespace mesaac::mol
