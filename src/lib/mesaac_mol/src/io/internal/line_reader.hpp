#pragma once
//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include <iostream>
#include <string>

namespace mesaac::mol::internal {

// Reads lines of text from a file.
struct LineReader {
  LineReader(std::istream &inf, const std::string &description);

  bool next(std::string &line);

  [[nodiscard]] std::string file_pos() const;

  [[nodiscard]] size_t linenum() const { return m_line_num; }

  [[nodiscard]] bool eof() const { return m_inf.eof() || m_inf.bad(); }

private:
  std::istream &m_inf;
  std::string m_description; // Description, e.g., filename, of inf.
  size_t m_line_num;
};

} // namespace mesaac::mol::internal
