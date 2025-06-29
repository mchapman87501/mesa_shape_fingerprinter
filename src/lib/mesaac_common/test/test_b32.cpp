// Unit test for b32
// Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <random>
#include <string>

#include "mesaac_common/b32.hpp"

using namespace std;

namespace mesaac::common {

namespace {
void roundtrip(string orig) {
  B32 codec;

  string encoded = codec.encode(orig);
  string decoded = codec.decode(encoded);
  REQUIRE(orig == decoded);
}

TEST_CASE("mesaac::common::B32", "[mesaac]") {

  SECTION("A basic test") {
    string orig;
    orig += (char)0xF0;
    orig += (char)0x0F;
    roundtrip(orig);
  }

  SECTION("Fuzz testing") {
    // For reproducibility:
    srandom(20250627);

    for (int i = 0; i < 160; i++) {
      string src = "";
      for (int j = 0; j < i; j++) {
        src += (char)(random() & 0xFF);
      }
      roundtrip(src);
      REQUIRE(src.size() == i);
    }
  }

  SECTION("Test with all byte values") {
    for (int i = 0; i < 256; i++) {
      string src = "";
      for (int j = 0; j < 256; j++) {
        src += (char)((i + j) & 0xFF);
      }
      roundtrip(src);
    }
  }
}

} // namespace
} // namespace mesaac::common
