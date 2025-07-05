#include <catch2/catch_test_macros.hpp>

#include "mesaac_arg_parser/value_converter.hpp"

namespace mesaac::arg_parser {
namespace {

TEST_CASE("mesaac::arg_parser::value_converter",
          "[mesaac][mesaac_arg_parser]") {
  SECTION("Test conversion to uppercase") {
    REQUIRE(value_converter::to_uppercase("not All lowerCase") ==
            "NOT ALL LOWERCASE");
  }

  SECTION("Test conversion to int") {
    std::optional<int> actual;
    value_converter::convert("32", actual);
    REQUIRE(actual.has_value());
    REQUIRE(actual.value() == 32);
  }

  SECTION("Test conversion to double") {
    std::optional<double> actual;
    value_converter::convert("1.2e9", actual);
    REQUIRE(actual.has_value());
    REQUIRE(actual.value() == 1.2e9);
  }
}
} // namespace
} // namespace mesaac::arg_parser
