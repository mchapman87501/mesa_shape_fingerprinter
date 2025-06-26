// Unit test for mol::Bond
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include "mesaac_mol/bond.hpp"

using namespace std;

namespace mesaac {
namespace mol {

TEST_CASE("mesaac::mol::Bond", "[mesaac]") {

  SECTION("Basic tests") {
    mol::Bond b;
    b.a0(1);
    b.a1(2);

    REQUIRE(b.a0() == 1);
    REQUIRE(b.a1() == 2);

    REQUIRE(b.type() == mol::Bond::BTE_SINGLE);
    b.type(mol::Bond::BTE_DOUBLE);
    REQUIRE(b.type() == mol::Bond::BTE_DOUBLE);

    b.stereo(mol::Bond::BSE_NOT_STEREO);
    REQUIRE(b.stereo() == mol::Bond::BSE_NOT_STEREO);

    b.optional_cols("xxxrrrccc");
    REQUIRE(b.optional_cols() == "xxxrrrccc");
  }
}
} // namespace mol
} // namespace mesaac
