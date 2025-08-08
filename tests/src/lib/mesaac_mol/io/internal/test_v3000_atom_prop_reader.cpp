#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "v3000_atom_prop_reader.hpp"

#include <format>
#include <sstream>
#include <vector>

namespace mesaac::mol::internal {
namespace {
TEST_CASE("mesaac::mol::internal::V3000AtomPropReader - General", "[mesaac]") {
  V3000AtomPropReader reader;

  // Read some randomly-chosen properties.
  // The value for ATTCHORD, here, is nonsense.  But it is from an RDKit
  // example.
  std::istringstream ins(
      R"LINES( CHG=-3 RAD=2 CFG=1 MASS=14.0032420 RGROUPS=(3 1 2 3) ATTCHORD=(4 8 Al 2 Br))LINES");
  const auto result = reader.read(ins);

  REQUIRE(result.is_ok());
  const auto value = result.value();
  REQUIRE(value.chg == -3);
  REQUIRE(value.rad == 2);
  REQUIRE(value.cfg == 1);
  REQUIRE_THAT(value.mass, Catch::Matchers::WithinAbs(14.0032420f, 1.0e-6));

  REQUIRE(value.rgroups == "(3 1 2 3)");

  REQUIRE(value.attchord == "(4 8 Al 2 Br)");

  // TODO verify other fields have default values.
}
} // namespace
} // namespace mesaac::mol::internal
