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
    Bond b(1, 2);

    REQUIRE(b.a0() == 1);
    REQUIRE(b.a1() == 2);

    REQUIRE(b.type() == BondType::bt_single);
    REQUIRE(b.stereo() == BondStereo::bs_not_stereo);

    Bond b2(1, 2, BondType::bt_double);
    REQUIRE(b2.type() == BondType::bt_double);

    Bond b3(1, 2, BondType::bt_double, BondStereo::bs_either);
    REQUIRE(b3.stereo() == BondStereo::bs_either);

    Bond b4(1, 2, BondType::bt_double, BondStereo::bs_not_stereo, "xxxrrrccc");
    REQUIRE(b4.optional_cols() == "xxxrrrccc");
  }
}
} // namespace
} // namespace mesaac::mol
