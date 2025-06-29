// Unit test for mol::Bond
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include "mesaac_mol/bond.hpp"

using namespace std;

namespace mesaac::mol {
namespace {
TEST_CASE("mesaac::mol::Bond", "[mesaac]") {

  SECTION("Basic tests") {
    mol::Bond b(1, 2);

    REQUIRE(b.a0() == 1);
    REQUIRE(b.a1() == 2);

    REQUIRE(b.type() == mol::Bond::BTE_SINGLE);
    REQUIRE(b.stereo() == mol::Bond::BSE_NOT_STEREO);

    mol::Bond b2(1, 2, mol::Bond::BTE_DOUBLE);
    REQUIRE(b2.type() == mol::Bond::BTE_DOUBLE);

    mol::Bond b3(1, 2, mol::Bond::BTE_DOUBLE, mol::Bond::BSE_EITHER);
    REQUIRE(b3.stereo() == mol::Bond::BSE_EITHER);

    mol::Bond b4(1, 2, mol::Bond::BTE_DOUBLE, mol::Bond::BSE_NOT_STEREO,
                 "xxxrrrccc");
    REQUIRE(b4.optional_cols() == "xxxrrrccc");
  }
}
} // namespace
} // namespace mesaac::mol
