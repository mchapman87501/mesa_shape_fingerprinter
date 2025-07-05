#include <catch2/catch_test_macros.hpp>

#include "mesaac_arg_parser/flag.hpp"

namespace mesaac::arg_parser {
namespace {

TEST_CASE("mesaac::arg_parser::Flag", "[mesaac][mesaac_arg_parser]") {
  SECTION("Parse options with flag") {
    Flag example("-e", "--example", "Show an example");

    REQUIRE(example.value() == false);
    CLIArgs short_args{"-e"};
    REQUIRE(short_args.size() == 1);
    REQUIRE(example.parse(short_args) == ParseResult::match());
    REQUIRE(example.value() == true);
    REQUIRE(short_args.empty());
  }

  SECTION("Get flag usage") {
    Flag example("-e", "--example", "Show an example");
    REQUIRE(example.usage() == "-e | --example");
    REQUIRE(example.help() == "-e | --example\n        Show an example");
  }

  SECTION("Parse options without a flag") {
    Flag example("-e", "--example", "Show an example");

    REQUIRE(example.value() == false);
    CLIArgs short_args{"positional", "args"};
    REQUIRE(short_args.size() == 2);
    REQUIRE(example.parse(short_args) == ParseResult::no_match());
    REQUIRE(example.value() == false);
    REQUIRE(short_args.size() == 2);
  }
}
} // namespace
} // namespace mesaac::arg_parser
