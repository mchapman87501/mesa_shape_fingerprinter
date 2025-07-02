// Unit test for mol::Atom
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mesaac_mol/atom.hpp"

using namespace std;

namespace mesaac::mol {

namespace {
TEST_CASE("mesaac::Atom", "[mesaac]") {
  SECTION("Basic tests") {
    Atom atom({.atomic_num = 6});
    REQUIRE(atom.atomic_num() == 6);

    REQUIRE_THAT(atom.pos().x(), Catch::Matchers::WithinAbs(0.0f, 1.0e-6));
    REQUIRE_THAT(atom.pos().y(), Catch::Matchers::WithinAbs(0.0f, 1.0e-6));
    REQUIRE_THAT(atom.pos().z(), Catch::Matchers::WithinAbs(0.0f, 1.0e-6));

    REQUIRE(atom.optional_cols() == "");
    REQUIRE(atom.symbol() == "C");
    REQUIRE(atom.radius() == 1.7f);
    REQUIRE(!atom.is_hydrogen());

    Atom hyd({.atomic_num = 1, .pos = Position(), .optional_cols = "fooo"});
    REQUIRE(hyd.symbol() == "H");
    REQUIRE(hyd.radius() == 1.09f);
    REQUIRE(hyd.is_hydrogen());
    REQUIRE(hyd.optional_cols() == "fooo");

    Atom oxy({.atomic_num = 8, .pos = Position(), .optional_cols = "bar"});
    REQUIRE(oxy.symbol() == "O");
    REQUIRE(!oxy.is_hydrogen());
    REQUIRE(oxy.optional_cols() == "bar");

    const Position expected(10, 11, -200.5);
    atom.set_pos(expected);
    REQUIRE(atom.pos().x() == expected.x());
    REQUIRE(atom.pos().y() == expected.y());
    REQUIRE(atom.pos().z() == expected.z());
  }

  SECTION("Invalid atomic number") {
    Atom atom({.atomic_num = 512});
    REQUIRE_THROWS_AS(atom.symbol(), invalid_argument);
  }
}
} // namespace
} // namespace mesaac::mol