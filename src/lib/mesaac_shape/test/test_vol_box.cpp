// Unit test for vol_box
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "mesaac_shape/vol_box.hpp"

using namespace std;

namespace mesaac {
using namespace shape;

// Use the test data directory specified by TEST_DATA_DIR preprocessor symbol.
const string test_data_dir(TEST_DATA_DIR);

struct TestFixture {
  void read_test_points(string pathname, PointList &points) {
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
      Point fv;
      fv.push_back(x);
      fv.push_back(y);
      fv.push_back(z);
      points.push_back(fv);
    }
    inf.close();
  }

  void read_default_sphere(PointList &points) {
    read_test_points("hamm_spheroid_10k_11rad.txt", points);
  }

  Point make_point(float x, float y, float z, float r) {
    Point result;
    result.push_back(x);
    result.push_back(y);
    result.push_back(z);
    result.push_back(r);
    return result;
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

  void get_brute_force_folded(BitVector &src, BitVector &dest,
                              unsigned int num_folds) {
    const unsigned int src_size = src.size();
    unsigned int folded_size = src_size / (1 << num_folds);
    dest.resize(folded_size);
    dest.reset();
    unsigned int i;
    for (i = 0; i != src_size; ++i) {
      if (src.test(i)) {
        dest.set(i % folded_size);
      }
    }
  }
};

TEST_CASE("VolBox Testing", "[mesaac]") {
  TestFixture fixture;
  PointList sphere;
  fixture.read_default_sphere(sphere);
  VolBox vb(sphere, 1.0);

  SECTION("Test Copying") {
    Point p(fixture.make_point(0, 0, 0, 22.0));

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
    VolBox vb_empty(empty, 1.0);

    for (float x = -10.0; x != 10.0; x += 1.0) {
      BitVector matches(0);
      Point p(fixture.make_point(x, x, x, 22.0));
      // What about proving that this does not clear any bits?
      // Ah, never mind.
      vb_empty.set_bits_for_one_sphere(p, matches, 0);
      REQUIRE(matches.empty());
    }
  }

  SECTION("Set bits for one non-empty sphere") {
    const float r = 5.0;
    float r_sphere = fixture.get_max_extent(sphere);
    unsigned int total = 0;
    for (float x = -15.0; x != 15.0; x += 1.0) {
      BitVector vb_matches(sphere.size()), brute_force_matches(sphere.size());
      Point p(fixture.make_point(x, x, x, r));
      vb.set_bits_for_one_sphere(p, vb_matches, 0);
      fixture.get_bits(sphere, x, x, x, r, brute_force_matches);
      REQUIRE(brute_force_matches == vb_matches);
      if (::fabs(x) > r_sphere) {
        REQUIRE(vb_matches.count() == 0);
      }
      total += vb_matches.count();
    }
    // Try to ensure that the tests matched at least some points.
    REQUIRE(total > 0);
  }

  SECTION("Set bits for multiple spheres.") {
    PointList center_spheres;
    const float r = 5.0;

    BitVector brute_force(sphere.size());
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back(fixture.make_point(x, x, x, r));
      fixture.get_bits(sphere, x, x, x, r, brute_force);
    }

    BitVector vb_matches;
    vb.set_bits_for_spheres(center_spheres, vb_matches, true, 0);
    REQUIRE(brute_force == vb_matches);
    REQUIRE(vb_matches.count() > 0);
    REQUIRE(vb_matches.count() <= sphere.size());
  }

  SECTION("TODO Write tests for custom sphere scaling") {}

  SECTION("Get points within spheres") {
    PointList center_spheres;
    const float r = 5.0;

    BitVector brute_force(sphere.size());
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back(fixture.make_point(x, x, x, r));
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
    unsigned int total_points = sphere.size();
    const float R = fixture.get_max_extent(sphere);

    float d_r = (R - 1.5) / 10.0;
    for (float r = 1.5; r <= R; r += d_r) {
      PointList centers;
      centers.push_back(fixture.make_point(0, 0, 0, r));

      PointList expected;
      fixture.get_contained_points(sphere, centers, expected);

      PointList contained;
      vb.get_points_within_spheres(centers, contained, 0);

      REQUIRE(fixture.get_max_extent(contained) <= r);
      REQUIRE(expected == contained);

      double expected_count = total_points * (r * r * r) / (R * R * R);
      REQUIRE_THAT(expected_count,
                   Catch::Matchers::WithinRel(contained.size(), 0.0475));
    }
  }

  SECTION("Test spherules at various locations within a VolBox") {
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
      centers.push_back(fixture.make_point(center, 0, 0, r));

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

    SECTION("No double counting of overlapping spherules") {
      // This is a simple test which demonstrates that,
      // if two spherules overlap, their corresponding cloud points
      // will not be double-counted.

      const Point atom(fixture.make_point(0, 0, 0, 1.7));

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
      REQUIRE(!contained1.empty());
    }
  }

  SECTION("Test setting VolBox bits with a fixed offset") {
    // TODO:  test get_points_within_spheres with a non-zero offset,
    // and with from_scratch = false.
    const Point atom(fixture.make_point(0, 0, 0, 1.7));

    PointList mol1;
    mol1.push_back(atom);

    const unsigned int num_cloud_points(vb.size());
    const unsigned int offset(10);
    BitVector bits1, bits2;
    bits1.resize(num_cloud_points);
    bits2.resize(num_cloud_points + offset);

    vb.set_bits_for_spheres(mol1, bits1, false, 0);
    vb.set_bits_for_spheres(mol1, bits2, false, offset);

    REQUIRE(0 < num_cloud_points);

    unsigned int i;
    for (i = 0; i != num_cloud_points; i++) {
      REQUIRE(bits1.test(i) == bits2.test(i + offset));
    }
    REQUIRE(bits1.count() == bits2.count());
  }

  SECTION("Test basic folded fingerprints") {
    PointList center_spheres;
    const float r = 5.0;
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back(fixture.make_point(x, x, x, r));
    }

    BitVector full_fp, unfolded;
    vb.set_bits_for_spheres(center_spheres, full_fp, true, 0);

    const unsigned int max_folds = 5;
    unsigned int num_folds;
    unsigned int folded_size = vb.size();
    for (num_folds = 0, folded_size = vb.size(); num_folds != max_folds;
         ++num_folds, folded_size /= 2) {
      BitVector folded;
      folded.resize(folded_size);
      folded.reset();
      vb.set_folded_bits_for_spheres(center_spheres, folded, num_folds, 0);
      REQUIRE(folded.count() > 0);

      // cout << "Folds: " << num_folds << " = " << folded << endl;

      BitVector folded_brute;
      fixture.get_brute_force_folded(full_fp, folded_brute, num_folds);
      REQUIRE(folded == folded_brute);
    }
  }
}
} // namespace mesaac
