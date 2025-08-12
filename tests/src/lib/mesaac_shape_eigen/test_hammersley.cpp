// Unit test for hammersley
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <fstream>

#include "mesaac_shape_eigen/hammersley.hpp"

using namespace std;

namespace mesaac::shape_eigen {

namespace {
void check_max_extents(const PointList &points, float xmin, float xmax,
                       float ymin, float ymax, float zmin, float zmax) {
  float axmin, axmax, aymin, aymax, azmin, azmax;
  if (points.size()) {
    axmin = axmax = points[0][0];
    aymin = aymax = points[0][1];
    azmin = azmax = points[0][2];
    for (const auto &p : points) {
      float x(p[0]), y(p[1]), z(p[2]);
      axmin = min(axmin, x);
      axmax = max(axmax, x);
      aymin = min(aymin, y);
      aymax = max(aymax, y);
      azmin = min(azmin, z);
      azmax = max(azmax, z);
    }

    const float err = 0.002;
    REQUIRE_THAT(xmin, Catch::Matchers::WithinAbs(axmin, err));
    REQUIRE_THAT(xmax, Catch::Matchers::WithinAbs(axmax, err));
    REQUIRE_THAT(ymin, Catch::Matchers::WithinAbs(aymin, err));
    REQUIRE_THAT(ymax, Catch::Matchers::WithinAbs(aymax, err));
    REQUIRE_THAT(zmin, Catch::Matchers::WithinAbs(azmin, err));
    REQUIRE_THAT(zmax, Catch::Matchers::WithinAbs(azmax, err));
  }
}

TEST_CASE("mesaac::shape_eigen::Hammersley - get_ellipsoid", "[mesaac]") {
  const unsigned int num_points = 10240;
  const float scale = 1.0;
  const float scale_sqr = scale * scale;
  const float a = 1.0, b = 0.5, c = 0.85; // Arbitrary spheroid squish

  PointList points;
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

TEST_CASE("mesaac::shape_eigen::Hammersley - get_cuboid", "[mesaac]") {
  // Lacking better ideas, how about a semi-random test for a
  // flattish volume?
  PointList points;
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
} // namespace mesaac::shape_eigen
