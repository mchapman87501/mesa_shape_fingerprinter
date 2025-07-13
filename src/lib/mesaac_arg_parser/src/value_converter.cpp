#include "mesaac_arg_parser/value_converter.hpp"

#include <algorithm>
#include <cctype>
#include <optional>
#include <stdexcept>
#include <string>

namespace mesaac::arg_parser::value_converter {

std::string to_uppercase(const std::string str) {
  std::string result(str);
  std::transform(result.begin(), result.end(), result.begin(),
                 [](unsigned c) { return std::toupper(c); });
  return result;
}

void convert(const std::string &value, std::optional<int> &result) {
  result = std::stoi(value);
}

void convert(const std::string &value, std::optional<unsigned long> &result) {
  // Apple clang version 17.0.0 (clang-1700.3.10.950):
  // stoul does not throw out_of_range, as expected of C++14 and later,
  // when converting a negative value such as "-42".
  // Hence this.
  long bare_result = std::stol(value);
  if (bare_result < 0) {
    throw std::out_of_range("invalid unsigned long " + value);
  }
  result = bare_result;
}

void convert(const std::string &value, std::optional<unsigned int> &result) {
  // Apple clang version 17.0.0 (clang-1700.3.10.950):
  // stoul does not throw out_of_range, as expected of C++14 and later,
  // when converting a negative value such as "-42".
  // Hence this trip through stoi.
  int bare_result = std::stoi(value);
  if (bare_result < 0) {
    throw std::out_of_range("invalid unsigned int " + value);
  }
  result = bare_result;
}

void convert(const std::string &value, std::optional<float> &result) {
  result = std::stof(value);
}

void convert(const std::string &value, std::optional<double> &result) {
  result = std::stod(value);
}

void convert(const std::string &value, std::optional<std::string> &result) {
  result = value;
}

void convert(const std::string &value,
             std::optional<std::filesystem::path> &result) {
  result = std::filesystem::path(value);
}

} // namespace mesaac::arg_parser::value_converter