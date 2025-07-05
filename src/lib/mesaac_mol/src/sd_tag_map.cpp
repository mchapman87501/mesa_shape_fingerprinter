#include "mesaac_mol/sd_tag_map.hpp"

#include <iostream>

namespace mesaac::mol {
using namespace std;

void SDTagMap::add_unparsed(const string &tag_line, const string &value) {
  if (find(tag_line) != end()) {
    cerr << "Warning: tag already exists: '" << tag_line
         << "'.  Overwriting with new value." << endl;
  }
  insert_or_assign(tag_line, value);
}

void SDTagMap::add(const string &name, const string &value) {
  string tag = ">  <" + name + ">";
  add_unparsed(tag, value);
}

} // namespace mesaac::mol
