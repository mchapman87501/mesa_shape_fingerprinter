
#include "v2000_field_read_fns.hpp"

namespace mesaac::mol::internal {

bool float_field(const std::string &line, unsigned int i_start,
                 unsigned int i_len, float &value) {
  try {
    value = std::stof(line.substr(i_start, i_len));
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

bool uint_field(const std::string &line, unsigned int i_start,
                unsigned int i_len, unsigned int &value) {
  try {
    // stoul does not reject negative numeric literals.
    // It wraps them.  Hence this.
    int s_value = std::stoi(line.substr(i_start, i_len));
    if (s_value < 0) {
      value = 0;
      return false;
    }

    value = s_value;
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

bool int_field(const std::string &line, unsigned int i_start,
               unsigned int i_len, int &value) {
  try {
    value = std::stoi(line.substr(i_start, i_len));
    return true;
  } catch (std::exception &e) {
    return false;
  }
}

int optional_int_field(const std::string &line, unsigned int i_start,
                       unsigned int i_len) {
  int result = 0;
  if (!int_field(line, i_start, i_len, result)) {
    return 0;
  }
  return result;
}

unsigned int optional_uint_field(const std::string &line, unsigned int i_start,
                                 unsigned int i_len) {
  int result = 0;
  if (!int_field(line, i_start, i_len, result)) {
    return 0;
  }
  if (result < 0) {
    // TODO throw an exception.
    return 0;
  }
  return result;
}

} // namespace mesaac::mol::internal
