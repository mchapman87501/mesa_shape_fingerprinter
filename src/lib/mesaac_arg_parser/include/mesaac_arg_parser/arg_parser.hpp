#pragma once

#include <algorithm>
#include <deque>
#include <filesystem>
#include <iostream>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

#include "mesaac_arg_parser/argument.hpp"
#include "mesaac_arg_parser/choice.hpp"
#include "mesaac_arg_parser/common_types.hpp"
#include "mesaac_arg_parser/flag.hpp"
#include "mesaac_arg_parser/i_argument.hpp"
#include "mesaac_arg_parser/i_option.hpp"
#include "mesaac_arg_parser/multi_valued_option.hpp"
#include "mesaac_arg_parser/option.hpp"

/**
 * @brief Argument parsing tools
 */
namespace mesaac::arg_parser {

/**
 * @brief Parses command-line arguments.
 */
class ArgParser {
public:
  /**
   * @brief Create a new instance.
   * @param options command-line flags and options to recognize
   * @param arguments positional arguments to recognize
   * @param description program description to print when showing usage
   */
  ArgParser(const std::vector<IOption::Ptr> options,
            const std::vector<IArgument::Ptr> arguments,
            const std::string &description)
      : ArgParser(options, arguments, description, std::cerr) {}

  /**
   * @brief Create a new instance.
   * @param options command-line flags and options to recognize
   * @param arguments positional arguments to recognize
   * @param description program description to print when showing usage
   * @param outs stream to which to write any usage or error messages
   */
  ArgParser(const std::vector<IOption::Ptr> options,
            const std::vector<IArgument::Ptr> arguments,
            const std::string &description, std::ostream &outs)
      : m_prog("<program>"), m_options(options), m_arguments(arguments),
        m_description(description),
        m_help("-h", "--help", "Show this help message and exit"),
        m_outs(outs) {}

  [[nodiscard]] int parse_args(int argc, const char **const argv);

  [[nodiscard]] bool usage_requested() const { return m_help.value(); };

  void show_usage(std::exception &e) const;
  void show_usage(const std::string &error_msg) const;
  void show_usage() const;

private:
  std::string m_prog;
  const std::string m_description;
  const std::vector<IOption::Ptr> m_options;
  const std::vector<IArgument::Ptr> m_arguments;
  std::ostream &m_outs;
  Flag m_help;

  [[nodiscard]] ParseResult parse_options(CLIArgs &args);
  [[nodiscard]] ParseResult parse_arguments(CLIArgs &args);
  [[nodiscard]] ParseResult check_unknown_flag_opt(CLIArgs &args);

  void report_unused_args(const CLIArgs &unused_args) const;
  [[nodiscard]] std::optional<std::string> missing_args_msg() const;
};

} // namespace mesaac::arg_parser