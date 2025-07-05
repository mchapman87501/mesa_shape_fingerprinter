#pragma once

#include <deque>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "mesaac_arg_parser/i_option.hpp"

namespace mesaac::arg_parser {
struct Flag : public IOption {
  using Ptr = std::shared_ptr<Flag>;

  static Ptr create(std::string_view short_name, std::string_view name,
                    std::string_view help = "") {
    return std::make_shared<Flag>(short_name, name, help);
  }

  Flag(std::string_view short_name, std::string_view name,
       std::string_view help = "")
      : m_short_name(short_name), m_name(name), m_help(help),
        m_flag_args{m_short_name, m_name}, m_parsed(false), m_is_set(false) {}

  [[nodiscard]] ParseResult parse(CLIArgs &remaining_args) {
    if (remaining_args.size() > 0) {
      for (const auto &flag : m_flag_args) {
        if (flag == remaining_args.front()) {
          remaining_args.pop_front();
          // Already seen?  Nag the user.
          if (m_is_set) {
            return ParseResult::match_with_error(flag +
                                                 " specified multiple times.");
          }
          m_is_set = true;
          m_parsed = true;
          return ParseResult::match();
        }
      }
    }
    return ParseResult::no_match();
  }

  [[nodiscard]] bool value() const { return m_is_set; }

  [[nodiscard]] std::string usage() const {
    std::ostringstream msg;
    std::string sep = "";
    for (const auto &flag : m_flag_args) {
      msg << sep << flag;
      sep = " | ";
    }
    return msg.str();
  }

  [[nodiscard]] std::string help() const {
    std::ostringstream msg;
    msg << usage() << std::endl << "        " << m_help;
    return msg.str();
  }

private:
  const std::string m_short_name;
  const std::string m_name;
  const std::string m_help;

  const std::vector<std::string> m_flag_args;

  bool m_parsed;
  bool m_is_set;
};

} // namespace mesaac::arg_parser