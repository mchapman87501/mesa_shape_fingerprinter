#pragma once

#include <deque>
#include <memory>
#include <string>

#include "mesaac_arg_parser/common_types.hpp"
#include "mesaac_arg_parser/parse_result.hpp"
namespace mesaac::arg_parser {
/**
 * @brief IOption defines the public interface for any command-line parameter
 * specification, such as a Flag, an Option, or a Choice.
 */
struct IOption {
  using Ptr = std::shared_ptr<IOption>;

  /**
   * @brief Try to match command-line arguments.
   * @param remaining_args arguments that haven't been matched yet
   * @return a ParseResult describing the outcome of the match attempt
   */
  [[nodiscard]] virtual ParseResult parse(CLIArgs &remaining_args) = 0;

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