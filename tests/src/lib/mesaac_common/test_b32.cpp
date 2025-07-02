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

  SECTION("Decode invalid B32 string") {
    // For a different approach see test_b64's section on decoding
    // corrupted B64 strings.
    B32 codec;
    for (unsigned char curr_char = 0; curr_char < 0xFF; ++curr_char) {
      // The B32 alphabet is not exposed to this test code.  Hence
      // some of this hardwired nonsense.
      // Verify that B32::decode throws for any character that is
      // neither a decimal numeral ("0"..."9") nor an uppercase latin letter
      // ("A"..."Z").
      const string bad_char_samples = "abcdefghijklmnopqrstuvwxyz_-+,";
      B32 codec;
      for (const auto &bad_char : bad_char_samples) {
        string input(16, bad_char);
        REQUIRE_THROWS_AS(codec.decode(input), runtime_error);
      }
    }
  }
}

} // namespace
} // namespace mesaac::common
