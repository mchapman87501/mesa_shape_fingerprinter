#pragma once

#include <deque>
#include <memory>
#include <string>

#include "mesaac_arg_parser/common_types.hpp"
#include "mesaac_arg_parser/parse_result.hpp"

namespace mesaac::arg_parser {
/**
 * @brief IArgument defines the public interface for any command-line positional
 * parameter.
 */
struct IArgument {
  using Ptr = std::shared_ptr<IArgument>;

  /**
   * @brief Try to match remaining command-line arguments.
   * @param remaining_args arguments that haven't been matched yet
   * @return a ParseResult describing the outcome of the match attempt
   */
  [[nodiscard]] virtual ParseResult parse(CLIArgs &remaining_args) = 0;

  /**
   * @brief Call this method after parsing to find out whether this parameter
   * has been matched to a command-line argument.
   * @return `true` iff this parameter has a parsed value
   */
  [[nodiscard]] virtual bool has_value() const = 0;

  /**
   * @brief Get the name of this parameter.
   * @return the parameter's name
   */
  [[nodiscard]] virtual std::string name() const = 0;

  /**
   * @brief Get a summary of command-line syntax.
   * @return a summary string suitable for inclusion in a "Usage:" message
   */
  [[nodiscard]] virtual std::string usage() const = 0;

  /**
   * @brief Get detailed information about this command-line parameter.
   * @return a string suitable for inclusion in a program's help message
   */
  [[nodiscard]] virtual std::string help() const = 0;
};

} // namespace mesaac::arg_parser