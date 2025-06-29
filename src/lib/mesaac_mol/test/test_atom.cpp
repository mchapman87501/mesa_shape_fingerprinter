// Unit test for mol::Atom
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include "mesaac_mol/atom.hpp"

using namespace std;

namespace mesaac::mol {

namespace {
TEST_CASE("mesaac::Atom", "[mesaac]") {
  SECTION("Basic tests") {
    Atom a(6);
    REQUIRE(a.atomic_num() == 6);

    REQUIRE(a.x() == 0.0f);
    REQUIRE(a.y() == 0.0f);
    REQUIRE(a.z() == 0.0f);

    REQUIRE(a.optional_cols() == "");
    REQUIRE(a.symbol() == "C");
    REQUIRE(a.radius() == 1.7f);
    REQUIRE(!a.is_hydrogen());

    Atom hyd(1, Position(), "fooo");
    REQUIRE(hyd.symbol() == "H");
    REQUIRE(hyd.radius() == 1.09f);
    REQUIRE(hyd.is_hydrogen());
    REQUIRE(hyd.optional_cols() == "fooo");

    Atom oxy(8, Position(), "bar");
    REQUIRE(oxy.symbol() == "O");
    REQUIRE(!oxy.is_hydrogen());
    REQUIRE(oxy.optional_cols() == "bar");

    a.set_pos(Position(10, 11, -200.5));
    REQUIRE(a.x() == 10.0f);
    REQUIRE(a.y() == 11.0f);
    REQUIRE(a.z() == -200.5f);
  }

  SECTION("Invalid atomic number") {
    Atom a(512);
    REQUIRE_THROWS_AS(a.symbol(), invalid_argument);
  }
}
} // namespace
} // namespace mesaac::mol