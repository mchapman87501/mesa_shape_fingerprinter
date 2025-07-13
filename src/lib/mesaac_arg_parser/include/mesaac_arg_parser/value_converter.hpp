#pragma once

#include <filesystem>
#include <optional>
#include <string>

namespace mesaac::arg_parser::value_converter {

[[nodiscard]] std::string to_uppercase(const std::string str);

void convert(const std::string &value, std::optional<int> &result);
void convert(const std::string &value, std::optional<unsigned long> &result);
void convert(const std::string &value, std::optional<unsigned int> &result);
void convert(const std::string &value, std::optional<float> &result);
void convert(const std::string &value, std::optional<double> &result);
void convert(const std::string &value, std::optional<std::string> &result);
void convert(const std::string &value,
             std::optional<std::filesystem::path> &result);
} // namespace mesaac::arg_parser::value_converter