#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "measure_type_converter.hpp"

namespace mesaac::cli::measures {
TEST_CASE("mesaac::cli::measures::get_measure_type", "[mesaac]") {
  SECTION("Test with invalid measure code") {
    REQUIRE_THROWS_AS(get_measure_type(' '), std::invalid_argument);
  }
  SECTION("Test with valid measure code") {
    REQUIRE(get_measure_type('B') == mesaac::measures::MeasureType::bub);
  }
}
} // namespace mesaac::cli::measures
