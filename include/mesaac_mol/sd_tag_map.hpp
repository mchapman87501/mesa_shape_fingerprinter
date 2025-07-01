#pragma once

#include <map>
#include <sstream>
#include <string>

namespace mesaac::mol {

class SDTagMap : public std::map<std::string, std::string> {
public:
  SDTagMap() {}

  void add(const std::string &name, const std::string &value);

  template <typename T> void add(const std::string &name, const T &value) {
    std::ostringstream outs;
    outs << value;
    add(name, outs.str());
  }

  void add_unparsed(const std::string &tag_line, const std::string &value);
};

} // namespace mesaac::mol