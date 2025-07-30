//
// Copyright (c) 2005-2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include "mesaac_mol/io/sdreader.hpp"

#include <format>
#include <sstream>

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

  bool read(Mol &next) {
    internal::MolHeaderBlock header;
    internal::CTab ctab;
    if (read_molfile(ctab)) {
      SDTagMap tags;
      if (read_tags(tags)) {
        next = Mol({
            .atoms = ctab.atoms,
            .bonds = ctab.bonds,
            .tags = tags,
            .name = ctab.name,
            .metadata = ctab.metadata,
            .comments = ctab.comments,
            .counts_line = ctab.counts_line,
            .properties_block = ctab.raw_properties_block,
        });
        return true;
      }
    }
    return false;
  }

  bool read_molfile(internal::CTab &ctab) {
    internal::MolHeaderBlock header;
    if (header.read(m_lines)) {
      return header.is_v3000() ? m_v3000.read(header, ctab)
                               : m_v2000.read(header, ctab);
    }
    return false;
  }

  bool read_tags(SDTagMap &tags) {
    internal::SDTagsReader tags_reader(m_lines);
    return tags_reader.read(tags);
  }

  // Skip the next mol.
  bool skip() {
    if (m_lines.eof()) {
      return false;
    }
    skip_to_end();
    return true;
  }

private:
  // Skip to the end of the current mol.
  void skip_to_end() {
    std::string line;
    while (m_lines.next(line)) {
      if (line == "$$$$") {
        return;
      }
    }
  }

  internal::LineReader m_lines;
  internal::V2000CTabReader m_v2000;
  internal::V3000CTabReader m_v3000;
};

// SDReader:
SDReader::SDReader(istream &inf, const string &pathname)
    : m_impl(std::make_unique<SDReaderImpl>(inf, pathname)) {}

SDReader::~SDReader() {}

bool SDReader::skip() { return m_impl->skip(); }

bool SDReader::read(Mol &next) { return m_impl->read(next); }
} // namespace mesaac::mol
