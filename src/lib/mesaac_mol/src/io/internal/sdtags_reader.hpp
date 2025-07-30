#pragma once
//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include "line_reader.hpp"

#include "mesaac_mol/sd_tag_map.hpp"
#include <string>

namespace mesaac::mol::internal {

struct SDTagsReader {
  SDTagsReader(LineReader &lines) : m_lines(lines) {}

  [[nodiscard]] bool read(SDTagMap &tags);

private:
  LineReader &m_lines;
};

} // namespace mesaac::mol::internal
