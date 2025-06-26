// Unit test for vol_box
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <fstream>
#include <iostream>

#include "mesaac_shape_eigen/vol_box.hpp"

using namespace std;

namespace mesaac {
namespace shape_eigen {
namespace {
struct TestFixture {
  void read_test_points(string pathname, PointList &points) {
    // Use the test data directory spec'd by TEST_DATA_DIR preprocessor symbol.
    const string test_data_dir(TEST_DATA_DIR);

    // TODO use std::filesystem::path, available since C++17.
    const string data_dir(test_data_dir + "/hammersley/");
    pathname = data_dir + pathname;
    points.clear();
    ifstream inf(pathname.c_str());
    if (!inf) {
      ostringstream msg;
      msg << "Could not open " << pathname << " for reading." << endl;
      throw std::runtime_error(msg.str());
    }
    float x, y, z;
    while (inf >> x >> y >> z) {
      points.push_back({x, y, z});
    }
    inf.close();
  }

  void read_default_sphere(PointList &points) {
    read_test_points("hamm_spheroid_10k_11rad.txt", points);
  }

  void get_bits(PointList &cloud, float x, float y, float z, float r,
                BitVector &result) {
    const float rsqr = r * r;
    for (unsigned int i = 0; i != cloud.size(); ++i) {
      Point &p(cloud[i]);
      float dx = p[0] - x, dy = p[1] - y, dz = p[2] - z;
      if (rsqr >= (dx * dx + dy * dy + dz * dz)) {
        result.set(i);
      }
    }
  }

  void get_contained_points(PointList &cloud, PointList &centers,
                            PointList &contained) {
    contained.clear();
    for (unsigned int i = 0; i != cloud.size(); i++) {
      Point &p_cloud(cloud[i]);
      for (unsigned int j = 0; j != centers.size(); j++) {
        Point &p_center(centers[j]);
        float dx = p_cloud[0] - p_center[0], dy = p_cloud[1] - p_center[1],
              dz = p_cloud[2] - p_center[2], r = p_center[3], rsqr = r * r,
              dssqr = (dx * dx + dy * dy + dz * dz);
        if (dssqr <= rsqr) {
          contained.push_back(p_cloud);
          break;
        }
      }
    }
  }

  float get_max_extent(PointList &points, bool verbose = false) {
    // Get the maximum distance of any point from the origin.
    float rsqr_max = 0.0;
    PointList::const_iterator i;
    if (verbose) {
      cout << "get_max_extent:" << endl;
    }
    for (i = points.begin(); i != points.end(); ++i) {
      const Point &p(*i);
      if (verbose) {
        cout << "    " << p[0] << ", " << p[1] << ", " << p[2] << endl;
      }
      float magsqr = (p[0] * p[0] + p[1] * p[1] + p[2] * p[2]);
      rsqr_max = (rsqr_max > magsqr) ? rsqr_max : magsqr;
    }
    if (verbose) {
      cout << "Result: " << ::sqrtf(rsqr_max) << endl;
    }
    return ::sqrtf(rsqr_max);
  }
};

} // namespace

TEST_CASE("mesaac::shape_eigen::VolBox", "[mesaac]") {
  TestFixture fixture;
  PointList sphere;
  fixture.read_default_sphere(sphere);
  VolBox vb(sphere, 1.0);

  SECTION("Test copying") {
    Point p{0, 0, 0, 22.0};

    {
      BitVector all_bits(10240);
      vb.set_bits_for_one_sphere(p, all_bits, 0);
      REQUIRE(all_bits.count() == 10240);
    }

    {
      VolBox vb2(vb);

      BitVector all_bits(10240);
      vb2.set_bits_for_one_sphere(p, all_bits, 0);
      REQUIRE(all_bits.count() == 10240);
    }
  }

  SECTION("Set bits for an empty sphere") {
    PointList empty;
    VolBox vb(empty, 1.0);

    for (float x = -10.0; x != 10.0; x += 1.0) {
      BitVector matches(0);
      Point p{x, x, x, 22.0};
      // What about proving that this does not clear any bits?
      // Ah, never mind.
      vb.set_bits_for_one_sphere(p, matches, 0);
      REQUIRE(matches.count() == 0);
    }
  }

  SECTION("Set bits for one non-empty sphere") {
    PointList sphere;
    fixture.read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    const float r = 5.0;
    float r_sphere = fixture.get_max_extent(sphere);
    unsigned int total = 0;
    for (float x = -15.0; x != 15.0; x += 1.0) {
      BitVector vb_matches(sphere.size()), brute_force_matches(sphere.size());
      Point p{x, x, x, r};
      vb.set_bits_for_one_sphere(p, vb_matches, 0);
      fixture.get_bits(sphere, x, x, x, r, brute_force_matches);
      REQUIRE(vb_matches == brute_force_matches);
      if (::fabs(x) > r_sphere) {
        REQUIRE(vb_matches.count() == 0);
      }
      total += vb_matches.count();
    }
    // Try to ensure that the tests matched at least some points.
    REQUIRE(total > 0);
  }

  SECTION("Set bits for multiple spheres.") {
    PointList sphere;
    fixture.read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    PointList center_spheres;
    const float r = 5.0;

    BitVector brute_force(sphere.size());
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back({x, x, x, r});
      fixture.get_bits(sphere, x, x, x, r, brute_force);
    }

    BitVector vb_matches;
    vb.set_bits_for_spheres(center_spheres, vb_matches, true, 0);
    REQUIRE(vb_matches == brute_force);
    REQUIRE(vb_matches.count() > 0);
    REQUIRE(vb_matches.count() <= sphere.size());
  }

  SECTION("TODO Write tests for custom sphere scaling") {}

  SECTION("Get points within spheres") {
    PointList sphere;
    fixture.read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    PointList center_spheres;
    const float r = 5.0;

    BitVector brute_force(sphere.size());
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back({x, x, x, r});
      fixture.get_bits(sphere, x, x, x, r, brute_force);
    }

    PointList bf_contained_points;
    fixture.get_contained_points(sphere, center_spheres, bf_contained_points);

    PointList contained_points;
    vb.get_points_within_spheres(center_spheres, contained_points, 0);
    REQUIRE(contained_points.size() > 0);
    REQUIRE(contained_points.size() == brute_force.count());

    // Not sure about the correctness of this test...
    REQUIRE(contained_points == bf_contained_points);
  }

  SECTION("Test VolBox containment for spheres of varying sizes.") {
    PointList sphere;
    fixture.read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);
    unsigned int total_points = sphere.size();
    const float R = fixture.get_max_extent(sphere);

    float d_r = (R - 1.5) / 10.0;
    for (float r = 1.5; r <= R; r += d_r) {
      PointList centers;
      centers.push_back({0, 0, 0, r});

      PointList expected;
      fixture.get_contained_points(sphere, centers, expected);

      PointList contained;
      vb.get_points_within_spheres(centers, contained, 0);

      REQUIRE(fixture.get_max_extent(contained) <= r);
      REQUIRE(expected == contained);

      float expected_count = total_points * (r * r * r) / (R * R * R);
      REQUIRE_THAT(expected_count,
                   Catch::Matchers::WithinRel(contained.size(), 0.05));
    }
  }

  SECTION("Test spherules at various locations within a VolBox") {
    PointList sphere;
    fixture.read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);
    unsigned int total_points = sphere.size();
    const float R = fixture.get_max_extent(sphere);
    const float r = 1.77; // Akin to carbon
    // How many cloud points to expect, based on density:
    const int exp_cnt = (int)(0.5 + total_points * (r * r * r) / (R * R * R));

    // Test various, fully-contained locations.
    const float max_offset = 0.9 * (R - r);
    const float d_center = max_offset / 10.0;
    for (float center = -max_offset; center <= max_offset; center += d_center) {
      PointList centers;
      centers.push_back({center, 0, 0, r});

      PointList expected;
      fixture.get_contained_points(sphere, centers, expected);

      PointList contained;
      vb.get_points_within_spheres(centers, contained, 0);

      float exp_max_extent = r + ::fabs(center);
      float act_max_extent = fixture.get_max_extent(contained);
      if (act_max_extent > exp_max_extent) {
        ostringstream msg;
        msg << "  Expected max extent: " << exp_max_extent << endl
            << "          Actual max: " << act_max_extent << endl;
        FAIL(msg.str());
      }
      REQUIRE(expected == contained);

      REQUIRE_THAT(exp_cnt,
                   Catch::Matchers::WithinRel(contained.size(), 0.075));
    }
  }

  SECTION("No double counting of overlapping spherules") {
    // This is a simple test which demonstrates that,
    // if two spherules overlap, their corresponding cloud points
    // will not be double-counted.

    const Point atom{0, 0, 0, 1.7};

    PointList mol1, mol2;
    mol1.push_back(atom);
    // Duplicates, superposed!
    mol2.push_back(atom);
    mol2.push_back(atom);

    PointList sphere;
    fixture.read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    PointList contained1, contained2;
    vb.get_points_within_spheres(mol1, contained1, 0);
    vb.get_points_within_spheres(mol2, contained2, 0);
    REQUIRE(contained1 == contained2);
    REQUIRE(contained1.size() > 0);
  }

  SECTION("Test setting VolBox bits with a fixed offset") {
    // TODO:  test get_points_within_spheres with a non-zero offset,
    // and with from_scratch = false.
    const Point atom{0, 0, 0, 1.7};

    PointList mol1;
    mol1.push_back(atom);

    PointList sphere;
    fixture.read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    const unsigned int num_cloud_points(vb.size());
    const unsigned int offset(10);
    BitVector bits1, bits2;
    bits1.resize(num_cloud_points);
    bits2.resize(num_cloud_points + offset);

    vb.set_bits_for_spheres(mol1, bits1, false, 0);
    vb.set_bits_for_spheres(mol1, bits2, false, offset);

    REQUIRE(num_cloud_points > 0);

    unsigned int i;
    for (i = 0; i != num_cloud_points; i++) {
      REQUIRE(bits2.test(i + offset) == bits1.test(i));
    }
    REQUIRE(bits2.count() == bits1.count());
  }
}
} // namespace shape_eigen
} // namespace mesaac
