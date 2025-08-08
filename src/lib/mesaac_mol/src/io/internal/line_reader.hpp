#pragma once
//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include "mesaac_mol/io/rw_result.hpp"
#include <iostream>
#include <string>

namespace mesaac::mol::internal {

// Reads lines of text from a file.
struct LineReader {
  LineReader(std::istream &inf, const std::string &description);

  RWResult next();

  /**
   * @brief Get a message annotated w. current file position.
   * @param msg_text the message to be annotated
   * @return the annotated message
   */
  [[nodiscard]] std::string message(const std::string &msg_text) const {
    return file_pos() + msg_text;
  }

  [[nodiscard]] std::string file_pos() const;

  [[nodiscard]] size_t linenum() const { return m_line_num; }

  [[nodiscard]] bool eof() const {
    return m_inf.eof() || m_inf.bad() || m_inf.fail();
  }

private:
  std::istream &m_inf;
  std::string m_description; // Description, e.g., filename, of inf.
  size_t m_line_num;
};

} // namespace mesaac::mol::internal
