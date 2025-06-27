// Unit test for gzip
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <random>
#include <string>

#include "mesaac_common/gzip.hpp"

using namespace std;

namespace mesaac {
namespace common {

namespace {
void roundtrip(string orig) {
  string compressed = gzip::compress(orig);
  string decompressed = gzip::decompress(compressed);
  REQUIRE(orig == decompressed);
}

} // namespace

TEST_CASE("mesaac::common::gzip", "[mesaac]") {

  SECTION("Basic round-trip") {
    string original("Some test, huh.");
    roundtrip(original);
  }

  SECTION("Fuzz test") {
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

  // SECTION("Fuzzy corruption") {
  //     // Too hard to test for corruption using random manglings.
  //     // If compressed data is corrupted, then sometimes you'll get
  //     // a runtime_error.  Sometimes you'll get no error, but an
  //     // inconsistent result.  Sometimes you'll get a correct result.
  // }
}

} // namespace common
} // namespace mesaac
