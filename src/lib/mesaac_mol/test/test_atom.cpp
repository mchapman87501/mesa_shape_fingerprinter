// Unit test for mol::Atom
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "mesaac_mol/atom.hpp"

using namespace std;

namespace mesaac {

TEST_CASE("mesaac::mol::Atom", "[mesaac]") {
  SECTION("Basic tests") {
    mol::Atom a;
    a.atomic_num(6);
    REQUIRE(a.atomic_num() == 6);

    REQUIRE(a.x() == 0.0f);
    REQUIRE(a.y() == 0.0f);
    REQUIRE(a.z() == 0.0f);

    REQUIRE(a.optional_cols() == "");
    REQUIRE(a.symbol() == "C");
    REQUIRE(a.radius() == 1.7f);
    REQUIRE(!a.is_hydrogen());

    a.atomic_num(1);
    a.optional_cols("fooo");
    REQUIRE(a.symbol() == "H");
    REQUIRE(a.radius() == 1.09f);
    REQUIRE(a.is_hydrogen());
    REQUIRE(a.optional_cols() == "fooo");

    a.atomic_num(8);
    REQUIRE(a.symbol() == "O");
    REQUIRE(!a.is_hydrogen());
    REQUIRE(a.optional_cols() == "fooo");

    a.x(10.0);
    a.y(11.0);
    a.z(-200.5);
    REQUIRE(a.x() == 10.0f);
    REQUIRE(a.y() == 11.0f);
    REQUIRE(a.z() == -200.5f);
  }

  SECTION("Invalid atomic number") {
    mol::Atom a;
    a.atomic_num(512);
    REQUIRE_THROWS_AS(a.symbol(), invalid_argument);
  }
}
} // namespace mesaac