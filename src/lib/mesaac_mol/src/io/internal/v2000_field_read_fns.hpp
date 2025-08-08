#pragma once

#include <string>

namespace mesaac::mol::internal {

bool float_field(const std::string &line, unsigned int i_start,
                 unsigned int i_len, float &value);
bool uint_field(const std::string &line, unsigned int i_start,
                unsigned int i_len, unsigned int &value);
bool int_field(const std::string &line, unsigned int i_start,
               unsigned int i_len, int &value);
int optional_int_field(const std::string &line, unsigned int i_start,
                       unsigned int i_len);
unsigned int optional_uint_field(const std::string &line, unsigned int i_start,
                                 unsigned int i_len);
} // namespace mesaac::mol::internal
