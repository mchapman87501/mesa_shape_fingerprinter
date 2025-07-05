#include <catch2/catch_test_macros.hpp>

#include "mesaac_arg_parser/choice.hpp"

namespace mesaac::arg_parser {
namespace {
TEST_CASE("mesaac::arg_parser::Choice", "[mesaac][mesaac_arg_parser]") {
  Choice choice("-c", "--choice", "Pick a value",
                {{"A", "value 1"}, {"B", "value 2"}, {"C", "value 3"}});

  SECTION("Test empty choice.") {
    REQUIRE_THROWS_AS(Choice("-c", "--choice", "No values!", {}),
                      std::invalid_argument);
  }

  SECTION("Test missing value.") {
    REQUIRE(choice.has_value() == false);
    CLIArgs args{"--choice"};
    const auto parse_result = choice.parse(args);
    REQUIRE(parse_result.matched());
    REQUIRE(parse_result.error_msg().has_value());
    REQUIRE(choice.has_value() == false);
  }

  SECTION("Test no match.") {
    CLIArgs args{"--signed", "-22"};
    REQUIRE(choice.parse(args) == ParseResult::no_match());
    REQUIRE(choice.has_value() == false);
    REQUIRE_THROWS_AS(choice.value(), std::bad_optional_access);
  }

  SECTION("Test valid choice.") {
    CLIArgs args{"--choice", "B"};
    const auto parse_result = choice.parse(args);
    REQUIRE(parse_result.matched_successfully());
    REQUIRE(choice.value() == "B");
  }

  SECTION("Test multiple appearances") {
    REQUIRE(choice.has_value() == false);
    CLIArgs args{"--choice", "B", "-c", "C"};
    REQUIRE(choice.parse(args) == ParseResult::match());
    REQUIRE(choice.has_value() == true);
    REQUIRE(choice.value() == "B");

    const auto parse_result_2 = choice.parse(args);
    REQUIRE(parse_result_2.matched());
    // Same option specified multiple times.
    REQUIRE(parse_result_2.error_msg().has_value());
  }

  SECTION("Get choice usage.") {
    REQUIRE(choice.usage() == "-c CHOICE | --choice CHOICE");
    REQUIRE(choice.help() == R"EXP(-c CHOICE | --choice CHOICE
        Pick a value
        valid values:
            A - value 1
            B - value 2
            C - value 3
)EXP");
  }
}
} // namespace
} // namespace mesaac::arg_parser