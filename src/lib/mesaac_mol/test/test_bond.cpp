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
    Bond bond(1, 2);

    REQUIRE(bond.a0() == 1);
    REQUIRE(bond.a1() == 2);

    REQUIRE(bond.type() == BondType::bt_single);
    REQUIRE(bond.stereo() == BondStereo::bs_not_stereo);

    Bond bond2(1, 2, BondType::bt_double);
    REQUIRE(bond2.type() == BondType::bt_double);

    Bond bond3(1, 2, BondType::bt_double, BondStereo::bs_either);
    REQUIRE(bond3.stereo() == BondStereo::bs_either);

    Bond bond4(1, 2, BondType::bt_double, BondStereo::bs_not_stereo,
               "xxxrrrccc");
    REQUIRE(bond4.optional_cols() == "xxxrrrccc");
  }
}
} // namespace
} // namespace mesaac::mol
