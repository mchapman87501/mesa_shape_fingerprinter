// Unit test for hammersley
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <fstream>

#include "mesaac_shape_eigen/hammersley.hpp"

using namespace std;

namespace mesaac {
namespace shape_eigen {

struct TestFixture {
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

  void check_point(Point &p, float x_exp, float y_exp, float z_exp) {
    REQUIRE_THAT(x_exp, Catch::Matchers::WithinAbs(p[0], 1.0e-3));
    REQUIRE_THAT(y_exp, Catch::Matchers::WithinAbs(p[1], 1.0e-3));
    REQUIRE_THAT(z_exp, Catch::Matchers::WithinAbs(p[2], 1.0e-3));
  }

  void spot_check(Point &p, int index, int num_points) {
    // Spot-check a few points against expected values generated
    // via a Python Hammersley implementation.
    if (0 == index) {
      check_point(p, 1.0 / num_points, 0.5, 1.0 / 3);
    } else if (5 == index) {
      check_point(p, 6.0 / num_points, 3.0 / 8, 2.0 / 9);
    } else if (10230 == index) {
      check_point(p, 10231.0 / num_points, 0.937073, 0.630747);
    } else if (10239 == index) {
      check_point(p, 10240.0 / num_points, 0.000305176, 0.569019);
    }
  }
};

TEST_CASE("mesaac::shape_eigen::Hammersley", "[mesaac]") {
  TestFixture fixture;

  SECTION("Basic tests") {
    Hammersley h;
    const int num_points = 10240;
    int num_generated = 0;

    h.start(num_points);
    Point p;
    while (h.next_point(p)) {
      REQUIRE(p.size() == 3);
      REQUIRE(p[0] == (num_generated + 1.0f) / num_points);
      // Can't think of any succinct tests for subsequent coords.
      REQUIRE(0 < p[1]);
      REQUIRE(p[1] <= 1.0);
      REQUIRE(0 < p[2]);
      REQUIRE(p[2] <= 1.0);

      fixture.spot_check(p, num_generated, num_points);
      num_generated++;
    }
    REQUIRE(num_generated == num_points);
  }

  SECTION("Test get_cubic") {
    // Lacking better ideas, how about a semi-random test for a
    // flattish volume?
    PointList points;
    float xmin = -2.0, xmax = 3.1, ymin = 5.0, ymax = 10.1, zmin = 0.0,
          zmax = 1.8;
    unsigned int num_points = 10240;
    Hammersley::get_cubic(xmin, xmax, ymin, ymax, zmin, zmax, num_points,
                          points);

    REQUIRE(points.size() == num_points);
    PointList::iterator i;
    for (i = points.begin(); i != points.end(); ++i) {
      Point &p(*i);
      REQUIRE(xmin <= p[0]);
      REQUIRE(p[0] <= xmax);
      REQUIRE(ymin <= p[1]);
      REQUIRE(p[1] <= ymax);
      REQUIRE(zmin <= p[2]);
      REQUIRE(p[2] <= zmax);
    }
    // Ensure the points fill the volume.
    fixture.check_max_extents(points, xmin, xmax, ymin, ymax, zmin, zmax);
  }
}

} // namespace shape_eigen
} // namespace mesaac
