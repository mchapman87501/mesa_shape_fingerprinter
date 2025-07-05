#pragma once

#include <deque>
#include <optional>
#include <string>

namespace mesaac::arg_parser {
/**
 * @brief A mutable sequence of command-line arguments.
 */
using CLIArgs = std::deque<std::string>;

/**
 * @brief An optional error message
 */
using OptErrMsg = std::optional<std::string>;

} // namespace mesaac::arg_parser