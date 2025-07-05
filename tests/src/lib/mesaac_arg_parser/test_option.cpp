#include <catch2/catch_test_macros.hpp>

#include "mesaac_arg_parser/option.hpp"

namespace mesaac::arg_parser {
namespace {
TEST_CASE("mesaac::arg_parser::Option", "[mesaac][mesaac_arg_parser]") {

  SECTION("Test positive integer option.") {
    // TODO with lower and upper bounds
    Option<unsigned int> opt("-u", "--unsigned", "Specify an unsigned value");
    REQUIRE(opt.has_value() == false);
    CLIArgs args{"--unsigned", "22"};
    REQUIRE(opt.parse(args) == ParseResult::match());
    REQUIRE(opt.has_value() == true);
    REQUIRE(opt.value() == 22);
    REQUIRE(args.empty());
  }

  SECTION("Test missing value.") {
    Option<unsigned int> opt("-u", "--unsigned", "Specify an unsigned value");
    REQUIRE(opt.has_value() == false);
    CLIArgs args{"--unsigned"};
    const auto parse_result = opt.parse(args);
    REQUIRE(parse_result.matched());
    REQUIRE(parse_result.error_msg().has_value());
    REQUIRE(opt.has_value() == false);
  }

  SECTION("Test no match.") {
    Option<unsigned int> opt("-u", "--unsigned", "Specify an unsigned value");
    CLIArgs args{"--signed", "-22"};
    REQUIRE(opt.parse(args) == ParseResult::no_match());
    REQUIRE(opt.has_value() == false);
    REQUIRE_THROWS_AS(opt.value(), std::bad_optional_access);
  }

  SECTION("Test character option.") {
    // TODO with a list of choices
    Option<std::string> opt("-v", "--value", "Specify a string value");
    std::string value("A multiword value");
    CLIArgs args{"-v", value};
    REQUIRE(opt.parse(args) == ParseResult::match());
    REQUIRE(opt.has_value() == true);
    REQUIRE(opt.value() == value);
  }

  SECTION("Test wrong value type") {
    Option<unsigned int> opt("-u", "--unsigned", "Specify an unsigned value");
    REQUIRE(opt.has_value() == false);
    CLIArgs args{"--unsigned", "not a number"};
    const auto parse_result = opt.parse(args);
    REQUIRE(parse_result.matched());
    REQUIRE(parse_result.error_msg().has_value());
    REQUIRE(opt.has_value() == false);
  }

  SECTION("Test multiple appearances") {
    Option<unsigned int> opt("-u", "--unsigned", "Specify an unsigned value");
    REQUIRE(opt.has_value() == false);
    CLIArgs args{"--unsigned", "2", "-u", "3"};
    REQUIRE(opt.parse(args) == ParseResult::match());
    REQUIRE(opt.has_value() == true);

    const auto parse_result_2 = opt.parse(args);
    REQUIRE(parse_result_2.matched());
    // Same option specified multiple times.
    REQUIRE(parse_result_2.error_msg().has_value());
  }

  SECTION("Get option usage.") {
    Option<std::string> opt("-v", "--value", "Specify a string value");
    REQUIRE(opt.usage() == "-v VALUE | --value VALUE");
    REQUIRE(opt.help() ==
            "-v VALUE | --value VALUE\n        Specify a string value");
  }
}
} // namespace
} // namespace mesaac::arg_parser