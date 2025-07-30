//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include "line_reader.hpp"

#include <format>

namespace mesaac::mol::internal {

LineReader::LineReader(std::istream &inf, const std::string &description)
    : m_inf(inf), m_description(description), m_line_num(0) {}

bool LineReader::next(std::string &line) {
  if (std::getline(m_inf, line)) {
    m_line_num += 1;
    return true;
  }
  return false;
}

std::string LineReader::file_pos() const {
  return std::format("File {}, line {}: ", m_description, m_line_num);
}
} // namespace mesaac::mol::internal
