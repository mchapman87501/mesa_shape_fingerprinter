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
  const auto result = lines.next();
  if (!result.is_ok()) {
    std::cerr << result.error() << std::endl;
    return false;
  }
  line = result.value();
  if (line.substr(0, 1) == ">") {
    std::string tag = line;
    std::ostringstream value;
    for (;;) {
      const auto result = lines.next();
      if (!result.is_ok()) {
        break;
      }
      line = result.value();
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

SDTagsReader::Result SDTagsReader::read() {
  SDTagMap tags;
  std::string line;
  while (read_one_tag(m_lines, tags, line)) {
    // loop
  }
  if (line != "$$$$") {
    return SDTagsReader::Result::Err(
        m_lines.message("Did not find $$$$ delimiter."));
  }
  return SDTagsReader::Result::Ok(tags);
}

} // namespace mesaac::mol::internal
