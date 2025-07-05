#pragma once

#include "mesaac_arg_parser/option_base.hpp"
#include <string>
#include <vector>

namespace mesaac::arg_parser {
/**
 * @brief A Choice is a command-line option with a fixed set of valid values.
 */
struct Choice : public OptionBase<std::string, 1> {
  using Ptr = std::shared_ptr<Choice>;

  /**
   * @brief Describes the valid options for a Choice.
   */
  struct ValidChoiceInfo {
    /**
     * @brief value to be supplied on the command line
     */
    const std::string value;

    /**
     * @brief description of the value
     */
    const std::string help;
  };

  using ValidChoices = std::vector<ValidChoiceInfo>;

  /**
   * @brief Create a new Choice.
   * @param short_name the short name, e.g., "-c"
   * @param name the long name, e.g., "--choice"
   * @param help description of this command-line option
   * @param valid_choices list of valid values
   * @return
   */
  static Ptr create(std::string_view short_name, std::string_view name,
                    std::string_view help, const ValidChoices &valid_choices) {
    return std::make_shared<Choice>(short_name, name, help, valid_choices);
  }

  /**
   * @brief Construct a Choice.
   * @param short_name the short name, e.g., "-c"
   * @param name the long name, e.g., "--choice"
   * @param help description of this command-line option
   * @param valid_choices list of valid values
   * @return
   */
  Choice(std::string_view short_name, std::string_view name,
         std::string_view help, const ValidChoices &valid_choices)
      : OptionBase<std::string, 1>(short_name, name, help),
        m_choices(valid_choices) {
    if (valid_choices.empty()) {
      throw std::invalid_argument(
          "At least one valid choice must be specified");
    }
  }

  [[nodiscard]] ParseResult parse(CLIArgs &remaining_args) override {
    ParseResult base_result = OptionBase<std::string, 1>::parse(remaining_args);
    if (base_result.matched_successfully()) {
      std::string value(m_value.value()[0]);
      for (const auto &curr_choice : m_choices) {
        if (curr_choice.value == value) {
          return base_result;
        }
      }
      return ParseResult::match_with_error("Invalid choice for '" + m_name +
                                           "': '" + value + "'");
    }
    return base_result;
  }

  [[nodiscard]] std::string help() const override {
    // Provide help on each choice.
    std::ostringstream outs;
    outs << OptionBase<std::string, 1>::help();
    outs << std::endl << "        valid values:" << std::endl;
    for (const auto &choice : m_choices) {
      outs << "            " << choice.value << " - " << choice.help
           << std::endl;
    }
    return outs.str();
  }

  [[nodiscard]] bool has_value() const { return this->m_value.has_value(); }
  [[nodiscard]] std::string value() const { return this->m_value.value()[0]; }
  [[nodiscard]] std::string value_or(const std::string &default_value) {
    if (this->m_value.has_value()) {
      return this->m_value.value()[0];
    }
    return default_value;
  }

private:
  const ValidChoices m_choices;
};
} // namespace mesaac::arg_parser