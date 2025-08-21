// Unit test for hammersley
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "mesaac_shape/hammersley.hpp"

using namespace std;

namespace mesaac::shape {

namespace {

void check_max_extents(const Point3DList &points, float xmin, float xmax,
                       float ymin, float ymax, float zmin, float zmax) {
  if (points.size()) {
    float axmin, axmax, aymin, aymax, azmin, azmax;
    axmin = axmax = points[0][0];
    aymin = aymax = points[0][1];
    azmin = azmax = points[0][2];
    for (const auto &point : points) {
      float x(point[0]), y(point[1]), z(point[2]);
      axmin = min(axmin, x);
      axmax = max(axmax, x);
      aymin = min(aymin, y);
      aymax = max(aymax, y);
      azmin = min(azmin, z);
      azmax = max(azmax, z);
    }

    const float err = 1.25e-3;
    REQUIRE_THAT(xmin, Catch::Matchers::WithinAbs(axmin, err));
    REQUIRE_THAT(xmax, Catch::Matchers::WithinAbs(axmax, err));
    REQUIRE_THAT(ymin, Catch::Matchers::WithinAbs(aymin, err));
    REQUIRE_THAT(ymax, Catch::Matchers::WithinAbs(aymax, err));
    REQUIRE_THAT(zmin, Catch::Matchers::WithinAbs(azmin, err));
    REQUIRE_THAT(zmax, Catch::Matchers::WithinAbs(azmax, err));
  }
}

TEST_CASE("mesaac::shape::Hammersley - get_ellipsoid", "[mesaac]") {
  const unsigned int num_points = 10240;
  const float scale = 1.0;
  const float scale_sqr = scale * scale;
  const float a = 1.0, b = 0.75, c = 0.35; // Arbitrary spheroid squish

  Point3DList points;
  Hammersley::get_ellipsoid(
      {.num_points = num_points, .scale = scale, .a = a, .b = b, .c = c},
      points);

  REQUIRE(!points.empty());
  REQUIRE(points.size() <= num_points);
  for (const auto &point : points) {
    REQUIRE(-1 <= point[0]);
    REQUIRE(point[0] <= 1);

    const auto mag = (point[0] * point[0] / a) + (point[1] * point[1] / b) +
                     (point[2] * point[2] / c);
    REQUIRE(mag > 0.0);
    REQUIRE(mag < scale_sqr);
  }
}

TEST_CASE("mesaac::shape::Hammersley - get_cuboid", "[mesaac]") {
  // Lacking better ideas, how about a semi-random test for a
  // flattish volume?
  Point3DList points;
  float xmin = -2.0, xmax = 3.1, ymin = 5.0, ymax = 10.1, zmin = 0.0,
        zmax = 1.8;
  unsigned int num_points = 10240;
  Hammersley::get_cuboid({.num_points = num_points,
                          .xmin = xmin,
                          .xmax = xmax,
                          .ymin = ymin,
                          .ymax = ymax,
                          .zmin = zmin,
                          .zmax = zmax},
                         points);

  REQUIRE(points.size() == num_points);
  for (const auto &point : points) {
    REQUIRE(xmin <= point[0]);
    REQUIRE(point[0] <= xmax);
    REQUIRE(ymin <= point[1]);
    REQUIRE(point[1] <= ymax);
    REQUIRE(zmin <= point[2]);
    REQUIRE(point[2] <= zmax);
  }
  // Ensure the points fill the volume.
  check_max_extents(points, xmin, xmax, ymin, ymax, zmin, zmax);
}

} // namespace
} // namespace mesaac::shape