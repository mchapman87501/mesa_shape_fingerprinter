#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mesaac_measures/bub.hpp"
#include <cmath>
#include <format>
#include <iostream>

namespace mesaac::measures {
TEST_CASE("mesaac::measures::BUB", "[mesaac]") {
  BUB measure;

  SECTION("Get name") { REQUIRE(measure.name() == "BUB"); }

  SECTION("Identical vectors") {
    shape_defs::BitVector v1(8, 0b11111111);

    REQUIRE(measure(v1, v1) == 1.0f);
  }

  SECTION("Completely different vectors") {
    shape_defs::BitVector v1(8, 0b11111111);
    shape_defs::BitVector v2(8, 0b00000000);

    REQUIRE(measure(v1, v2) == 0.0f);
  }

  SECTION("Exhaustive n-bit vectors") {
    const unsigned int num_bits = 5;
    const unsigned int value_max = 0b11111;

    const float min_similarity = 0.0f;
    const float max_similarity = 1.0f;

    for (unsigned int v1 = 0; v1 <= value_max; ++v1) {
      shape_defs::BitVector vec1(num_bits, v1);
      for (unsigned int v2 = v1; v2 <= value_max; ++v2) {
        shape_defs::BitVector vec2(num_bits, v2);

        const float similarity = measure(vec1, vec2);
        REQUIRE(similarity >= min_similarity);
        REQUIRE(similarity <= max_similarity);
      }
    }
  }
}
} // namespace mesaac::measures
