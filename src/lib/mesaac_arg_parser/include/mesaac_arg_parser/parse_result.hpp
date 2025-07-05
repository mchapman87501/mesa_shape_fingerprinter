#pragma once

#include "mesaac_arg_parser/common_types.hpp"

namespace mesaac::arg_parser {
/**
 * @brief Describes the result of parsing a command-line option or positional
 * parameter.
 */
struct ParseResult {
  /**
   * @brief Construct a new instance, describing the result of parsing a
   * command-line paramter, such as a Flag, an Option or a positional Argument.
   * @param matched whether or not the parameter was recognized
   * @param error_msg a description of any error that occurred while parsing
   * @note It's probably better to use one of the convenience methods -
   * ::no_match, ::match_with_error, or ::match.
   */
  ParseResult(bool matched, OptErrMsg error_msg)
      : m_matched(matched), m_error_msg(error_msg) {}

  /**
   * @brief Create a ParseResult that indicates no match was found.
   */
  static ParseResult no_match() { return {false, {}}; }

  /**
   * @brief Create a ParseResult to describe a malformed match.  For example,
   * this could be used to indicate than an Option's short or long name was
   * recognized, but that the required Option value was missing.
   * @param error_msg a description of the error that occurred while parsing
   */
  static ParseResult match_with_error(std::string_view error_msg) {
    return {true, std::string(error_msg)};
  }

  /**
   * @brief Create a ParseResult describing a succesful match.
   */
  static ParseResult match() { return {true, {}}; }

  /**
   * @brief Compare this ParseResult with `other`.
   * @param other another ParseResult
   * @return true iff this instance and other are structurally equivalent
   */
  bool operator==(const ParseResult &other) const {
    return m_matched == other.m_matched && m_error_msg == other.m_error_msg;
  }

  /**
   * @brief Find out whether this ParseResult was matched to a command-line
   * argument.
   * @return `true` if matched, `false` otherwise
   */
  bool matched() const { return m_matched; }

  /**
   * @brief Get a description of any parsing errors.
   * @return an optional string describing any parsing errors
   */
  OptErrMsg error_msg() const { return m_error_msg; }

  /**
   * @brief Find out whether this ParseResult represents an error-free match.
   */
  bool matched_successfully() const {
    return m_matched && !m_error_msg.has_value();
  }

private:
  bool m_matched;
  OptErrMsg m_error_msg;
};

} // namespace mesaac::arg_parser