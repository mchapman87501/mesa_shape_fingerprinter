// Unit test for b64
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <random>
#include <stdexcept>
#include <string>

#include "mesaac_common/b64.hpp"

using namespace std;

namespace mesaac::common {

namespace {
void roundtrip(string orig) {
  B64 codec;

  string encoded = codec.encode(orig);
  string decoded = codec.decode(encoded);
  REQUIRE(orig == decoded);
}

char random_char() {
  // Get a random character which is not a valid B64 char.
  static string alphabet =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  char result = '_';
  if ((random() & 0xFF) >= 0x10) {
    const int MAX = (int)('+') - 1;
    result = (char)(random() % MAX);
  } else {
    const int MIN = (int)('z') + 1;
    const int MAX = 255;
    result = (char)(MIN + (random() % (MAX - MIN)));
  }
  return result;
}

TEST_CASE("mesaac::common::B64", "[mesaac]") {

  SECTION("Basic test") {
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

  SECTION("Decode corrupted B64 strings") {
    // Fuzz test, decoding corrupted B64 strings.
    // In this case "corrupted" means only that the string
    // contains at least one invalid B64 character.
    B64 codec;
    const int LENGTH = 160;

    // For reproducibility:
    srandom(20250627);

    for (int i = 1; i < LENGTH; i++) {
      string src = "";
      for (int j = 0; j < i; j++) {
        src += (char)(random() & 0xFF);
      }

      string encoded = codec.encode(src);
      // Append or replace a single character of the encoded
      // string w. an invalid B64 character.
      int byte_to_munge = (random() % i);
      encoded[byte_to_munge] = random_char();
      REQUIRE_THROWS_AS(codec.decode(encoded), std::invalid_argument);
    }
  }
}

} // namespace
} // namespace mesaac::common
