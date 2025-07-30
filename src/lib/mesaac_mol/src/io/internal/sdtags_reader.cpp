//
// Copyright (c) 2025 Mesa Analytics & Computing, LLC
//

#include "sdtags_reader.hpp"

#include <iostream>
#include <string>

namespace mesaac::mol::internal {

namespace {

bool is_blank(std::string &line) {
  return (line.find_first_not_of(" \t") == std::string::npos);
}

bool read_one_tag(LineReader &lines, SDTagMap &tags, std::string &line) {
  if (lines.eof()) {
    return false;
  }

  lines.next(line);
  if (line.substr(0, 1) == ">") {
    std::string tag = line;
    std::ostringstream value;
    while (lines.next(line)) {
      if (is_blank(line)) {
        break;
      } else {
        value << line << std::endl;
      }
    }
    // TODO:  Extract the actual tag, distinguishing between
    // <TAG_NAME>, DTn field numbers and registry numbers
    tags.add_unparsed(tag, value.str());
    return true;
  }
  return false;
}

} // namespace

bool SDTagsReader::read(SDTagMap &tags) {
  bool result = true;
  std::string line;
  while (read_one_tag(m_lines, tags, line)) {
    // loop
  }
  if (line != "$$$$") {
    // TODO:  Exceptions
    std::cerr << m_lines.file_pos() << "Did not find SD $$$$ delimiter."
              << std::endl;
    result = false;
  }
  return result;
}

} // namespace mesaac::mol::internal
