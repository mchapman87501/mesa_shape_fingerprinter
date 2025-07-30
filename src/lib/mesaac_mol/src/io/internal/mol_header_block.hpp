#pragma once
//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include <string>

#include "line_reader.hpp"

namespace mesaac::mol::internal {

struct MolHeaderBlock {

  MolHeaderBlock()
      : m_name(""), m_metadata(""), m_comments(""), m_counts_line("") {}

  bool read(LineReader &lines) {
    return (lines.next(m_name) && lines.next(m_metadata) &&
            lines.next(m_comments) && lines.next(m_counts_line));
  }

  const std::string &name() const { return m_name; }
  const std::string &metadata() const { return m_metadata; }
  const std::string &comments() const { return m_comments; }
  const std::string &counts_line() const { return m_counts_line; }

  bool is_v3000() const {
    // TODO verify whether the CTfile spec is case-sensitive.
    return m_counts_line.ends_with("V3000");
  }

private:
  std::string m_name;
  std::string m_metadata;
  std::string m_comments;
  // The counts line isn't really part of the header block, but it is
  // included here for convenience.
  std::string m_counts_line;
};
} // namespace mesaac::mol::internal
