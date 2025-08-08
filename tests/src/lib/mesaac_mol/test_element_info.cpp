#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mesaac_mol/element_info.hpp"

#include <string>
#include <vector>

namespace mesaac::mol {

namespace {

TEST_CASE("mesaac::mol::element_info - basic queries", "[mesaac]") {
  const auto symbol = "C";
  const int atomic_num = 6;

  REQUIRE(get_atomic_num(symbol) == atomic_num);
  REQUIRE(get_symbol(atomic_num) == symbol);
  REQUIRE_THAT(get_atomic_mass(atomic_num),
               Catch::Matchers::WithinAbs(12.011, 1.0e-6));
  REQUIRE_THAT(get_radius(atomic_num),
               Catch::Matchers::WithinAbs(get_symbol_radius(symbol), 1.0e-6));
  REQUIRE_THAT(get_radius(atomic_num),
               Catch::Matchers::WithinAbs(1.70f, 1.0e-6));
}

TEST_CASE("mesaac::mol::element_info - bad atomic number", "[mesaac]") {
  const std::vector<int> bad_nums{-1, 0, 5000};

  for (const auto atomic_num : bad_nums) {
    REQUIRE_THROWS_AS(get_symbol(atomic_num), std::out_of_range);
    REQUIRE_THROWS_AS(get_atomic_mass(atomic_num), std::out_of_range);
    REQUIRE_THROWS_AS(get_radius(atomic_num), std::out_of_range);
  }

  const std::vector<std::string> bad_symbols{"Gum", "UnknownElement", "   "};

  for (const auto symbol : bad_symbols) {
    REQUIRE_THROWS_AS(get_atomic_num(symbol), std::invalid_argument);
    REQUIRE_THROWS_AS(get_symbol_radius(symbol), std::invalid_argument);
  }
}

} // namespace
} // namespace mesaac::mol