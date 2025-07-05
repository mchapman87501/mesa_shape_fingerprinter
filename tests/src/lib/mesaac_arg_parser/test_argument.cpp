#include <catch2/catch_test_macros.hpp>

#include "mesaac_arg_parser/argument.hpp"

namespace mesaac::arg_parser {
namespace {
TEST_CASE("mesaac::arg_parser::Argument", "[mesaac][mesaac_arg_parser]") {

  SECTION("Test string.") {
    Argument<std::string> arg("string_val", "Specify a string value");
    REQUIRE(arg.has_value() == false);
    REQUIRE_THROWS_AS(arg.value(), std::bad_optional_access);
    CLIArgs args{"a string value"};
    REQUIRE(arg.parse(args) == ParseResult::match());
    REQUIRE(arg.has_value() == true);
    REQUIRE(arg.value() == "a string value");
    REQUIRE(args.empty());
  }

  SECTION("Test float") {
    Argument<float> arg("some_arg", "Specify a float value");
    CLIArgs args{"21.5"};
    REQUIRE(arg.parse(args) == ParseResult::match());
    REQUIRE(arg.has_value() == true);
    REQUIRE(arg.value() == 21.5);
    REQUIRE(args.size() == 0);
  }

  SECTION("Test wrong value type.") {
    Argument<unsigned int> arg("some_arg", "Specify an unsigned value");
    CLIArgs args{"not a number"};
    auto parse_result = arg.parse(args);
    REQUIRE(parse_result.matched());
    REQUIRE(parse_result.error_msg().has_value());
    REQUIRE(arg.has_value() == false);
    REQUIRE_THROWS_AS(arg.value(), std::bad_optional_access);
    REQUIRE(args.size() == 1);
  }

  SECTION("Test unsigned int - negative value") {
    Argument<unsigned int> arg("value", "Specify an unsigned value");
    CLIArgs args{"-42"};

    auto parse_result = arg.parse(args);
    REQUIRE(parse_result.matched());
    REQUIRE(parse_result.error_msg().has_value());

    REQUIRE(arg.has_value() == false);
    REQUIRE(args.size() == 1);
  }

  SECTION("Test unsigned long - negative value") {
    Argument<unsigned long> arg("value", "Specify an unsigned value");
    CLIArgs args{"-42"};

    auto parse_result = arg.parse(args);
    REQUIRE(parse_result.matched());
    REQUIRE(parse_result.error_msg().has_value());

    REQUIRE(arg.has_value() == false);
    REQUIRE(args.size() == 1);
  }

  SECTION("Test parses value only once") {
    Argument<unsigned int> arg("value", "Specify an unsigned value");
    CLIArgs args{"42", "21"};

    REQUIRE(arg.parse(args) == ParseResult::match());
    REQUIRE(arg.parse(args) == ParseResult::no_match());
    REQUIRE(arg.value() == 42);
    REQUIRE(args.size() == 1);
  }

  SECTION("Get positional usage.") {
    Argument<std::string> arg("value", "Specify a string value");
    REQUIRE(arg.usage() == "value");
    REQUIRE(arg.help() == "value\n        Specify a string value");
  }
}
} // namespace
} // namespace mesaac::arg_parser