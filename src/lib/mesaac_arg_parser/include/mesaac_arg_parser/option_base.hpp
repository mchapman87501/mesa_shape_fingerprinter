#pragma once

#include <memory>
#include <sstream>
#include <stdexcept>

#include "mesaac_arg_parser/common_types.hpp"
#include "mesaac_arg_parser/i_option.hpp"
#include "mesaac_arg_parser/value_converter.hpp"

namespace mesaac::arg_parser {
template <typename ValueType, size_t NumValues>
struct OptionBase : public IOption {
  using Ptr = std::shared_ptr<OptionBase<ValueType, NumValues>>;
  static Ptr create(std::string_view short_name, std::string_view name,
                    std::string_view help) {
    return std::make_shared<OptionBase>(short_name, name, help);
  }

  OptionBase(std::string_view short_name, std::string_view name,
             std::string_view help)
      : m_short_name(short_name), m_name(name), m_help(help),
        m_flag_args{m_short_name, m_name} {}

  [[nodiscard]] ParseResult parse(CLIArgs &remaining_args) override {
    if (remaining_args.size() > 0) {
      for (const auto &flag : m_flag_args) {
        if (flag == remaining_args.front()) {
          // Already parsed this flag?
          if (m_value.has_value()) {
            return ParseResult::match_with_error(flag +
                                                 " specified multiple times");
          }

          // All required values present?
          if (remaining_args.size() < (1 + NumValues)) {
            std::ostringstream msg;
            msg << flag << " requires " << NumValues << " value";
            if (NumValues > 1) {
              msg << "s";
            }
            return ParseResult::match_with_error(msg.str());
          }
          remaining_args.pop_front();

          std::array<ValueType, NumValues> parsed_values;
          for (size_t i = 0; i < NumValues; ++i) {
            const auto sval(remaining_args.front());
            try {
              std::optional<ValueType> tmp_val;
              value_converter::convert(sval, tmp_val);
              parsed_values[i] = tmp_val.value();
              remaining_args.pop_front();
            } catch (std::exception &e) {
              std::ostringstream msg;
              msg << "Could not parse value " << i << " ('" << sval << "') of "
                  << flag << ": " << e.what();
              return ParseResult::match_with_error(msg.str());
            }
          }
          m_value = parsed_values;
          return ParseResult::match();
        }
      }
    }
    return ParseResult::no_match();
  }

  [[nodiscard]] std::string usage() const override {
    std::ostringstream msg;
    // Presume that m_name starts with "--".
    std::string upname = value_converter::to_uppercase(m_name.substr(2));
    std::string sep = "";
    for (const auto &flag : m_flag_args) {
      msg << sep << flag << " " << upname;
      sep = " | ";
    }
    return msg.str();
  }

  [[nodiscard]] std::string help() const override {
    std::ostringstream msg;
    msg << usage() << std::endl << "        " << m_help;
    return msg.str();
  }

protected:
  static constexpr size_t s_num_values = NumValues;
  const std::string m_short_name;
  const std::string m_name;
  const std::string m_help;

  std::vector<std::string> m_flag_args;
  std::optional<std::array<ValueType, NumValues>> m_value;
};

} // namespace mesaac::arg_parser