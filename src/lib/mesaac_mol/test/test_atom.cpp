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

    REQUIRE_THAT(atom.x(), Catch::Matchers::WithinAbs(0.0f, 1.0e-6));
    REQUIRE_THAT(atom.y(), Catch::Matchers::WithinAbs(0.0f, 1.0e-6));
    REQUIRE_THAT(atom.z(), Catch::Matchers::WithinAbs(0.0f, 1.0e-6));

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

    atom.set_pos(Position(10, 11, -200.5));
    REQUIRE(atom.x() == 10.0f);
    REQUIRE(atom.y() == 11.0f);
    REQUIRE(atom.z() == -200.5f);
  }

  SECTION("Invalid atomic number") {
    Atom atom({.atomic_num = 512});
    REQUIRE_THROWS_AS(atom.symbol(), invalid_argument);
  }
}
} // namespace
} // namespace mesaac::mol