#pragma once
//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include <string>

#include "line_reader.hpp"
#include "mesaac_mol/result.hpp"

namespace mesaac::mol::internal {

struct MolHeaderBlock {

  using Result = mesaac::mol::Result<MolHeaderBlock>;

  MolHeaderBlock(const std::string &name, const std::string &metadata,
                 const std::string comments, const std::string counts_line)
      : m_name(name), m_metadata(metadata), m_comments(comments),
        m_counts_line(counts_line) {}

  static Result read(LineReader &lines) {
    try {
      const std::string name = lines.next().value();
      const std::string metadata = lines.next().value();
      const std::string comments = lines.next().value();
      const std::string counts_line = lines.next().value();
      return Result::Ok(MolHeaderBlock(name, metadata, comments, counts_line));
    } catch (std::bad_optional_access &e) {
      return Result::Err(lines.message("Could not read MolHeaderBlock: " +
                                       std::string(e.what())));
    }
    return Result::Err(lines.message("Could not read MolHeaderBlock"));
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
  const std::string m_name;
  const std::string m_metadata;
  const std::string m_comments;
  // The counts line isn't really part of the header block, but it is
  // included here for convenience.
  const std::string m_counts_line;
};
} // namespace mesaac::mol::internal
