#pragma once

#include "mesaac_arg_parser/common_types.hpp"
#include "mesaac_arg_parser/i_argument.hpp"
#include "mesaac_arg_parser/value_converter.hpp"

#include <sstream>

namespace mesaac::arg_parser {
// TODO support multiple consecutive values.
template <typename ValueType> struct Argument : public IArgument {
  using Ptr = std::shared_ptr<Argument>;

  static Ptr create(std::string_view name, std::string_view help) {
    return std::make_shared<Argument>(name, help);
  }

  Argument(std::string_view name, std::string_view help)
      : m_name(name), m_help(help) {}

  [[nodiscard]] ParseResult parse(CLIArgs &remaining_args) override {
    if (m_value.has_value()) {
      return ParseResult::no_match();
    }

    if (remaining_args.size() > 0) {
      const auto value(remaining_args.front());
      try {
        value_converter::convert(value, m_value);
        remaining_args.pop_front();
        return ParseResult::match();
      } catch (std::exception &e) {
        std::ostringstream msg;
        msg << "Invalid literal value for " << m_name << ": '" << value << "'";
        return ParseResult::match_with_error(msg.str());
      }
    }
    return ParseResult::no_match();
  }

  [[nodiscard]] std::string name() const override { return m_name; }

  [[nodiscard]] std::string usage() const override { return m_name; }

  [[nodiscard]] std::string help() const override {
    std::ostringstream msg;

    msg << m_name << std::endl << "        " << m_help;
    return msg.str();
  }

  [[nodiscard]] bool has_value() const override { return m_value.has_value(); }

  /**
   * @brief Call this method after parsing to get the value of the command-line
   * argument to which this parameter was matched.
   * @return the matched command-line argument value
   * @throws if no command-line argument has been matched, this method will
   * throw std::bad_optional_access.
   */
  [[nodiscard]] ValueType value() const { return m_value.value(); }

private:
  const std::string m_name;
  const std::string m_help;

  std::optional<ValueType> m_value;
};

} // namespace mesaac::arg_parser