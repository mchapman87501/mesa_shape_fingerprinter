// Unit test for vol_box
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <filesystem>
#include <format>
#include <fstream>
#include <iostream>

#include "mesaac_shape/vol_box.hpp"

using namespace std;

namespace mesaac::shape {

namespace {
void read_test_points(filesystem::path filename, Point3DList &points) {
  // Use the test data directory spec'd by TEST_DATA_DIR preprocessor symbol.
  const filesystem::path test_data_dir(TEST_DATA_DIR);

  const auto data_dir(test_data_dir / "hammersley");
  const auto pathname = data_dir / filename;
  points.clear();
  ifstream inf(pathname);
  if (!inf) {
    throw runtime_error(
        format("Could not open {} for reading.", string(pathname)));
  }
  float x, y, z;
  while (inf >> x >> y >> z) {
    points.push_back({x, y, z});
  }
  inf.close();
}

void read_default_point_cloud(Point3DList &points) {
  read_test_points("hamm_spheroid_10k_11rad.txt", points);
}

void get_bits(Point3DList &cloud, float x, float y, float z, float r,
              shape_defs::BitVector &result) {
  const float rsqr = r * r;
  for (unsigned int i = 0; i != cloud.size(); ++i) {
    Point3D &p(cloud[i]);
    float dx = p[0] - x, dy = p[1] - y, dz = p[2] - z;
    if (rsqr >= (dx * dx + dy * dy + dz * dz)) {
      result.set(i);
    }
  }
}

void get_contained_points(const Point3DList &cloud, const SphereList &centers,
                          Point3DList &contained) {
  contained.clear();
  for (unsigned int i = 0; i != cloud.size(); i++) {
    const Point3D &p_cloud(cloud[i]);
    for (unsigned int j = 0; j != centers.size(); j++) {
      const Sphere &p_center(centers[j]);
      const float dx = p_cloud[0] - p_center[0], dy = p_cloud[1] - p_center[1],
                  dz = p_cloud[2] - p_center[2], r = p_center[3], rsqr = r * r,
                  dssqr = (dx * dx + dy * dy + dz * dz);
      if (dssqr <= rsqr) {
        contained.push_back(p_cloud);
        break;
      }
    }
  }
}

float get_max_extent(Point3DList &points) {
  // Get the maximum distance of any point from the origin.
  float rsqr_max = 0.0;
  for (const auto &point : points) {
    const float magsqr =
        (point[0] * point[0] + point[1] * point[1] + point[2] * point[2]);
    rsqr_max = max(rsqr_max, magsqr);
  }
  return ::sqrtf(rsqr_max);
}

void get_brute_force_folded(shape_defs::BitVector &src,
                            shape_defs::BitVector &dest,
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

TEST_CASE("mesaac::shape::VolBox", "[mesaac]") {
  Point3DList sphere;
  read_default_point_cloud(sphere);
  VolBox vb(sphere, 1.0);

  SECTION("Test Copying") {
    Sphere p{0, 0, 0, 22.0};

    {
      shape_defs::BitVector all_bits(10240);
      vb.set_bits_for_one_sphere(p, all_bits, 0);
      REQUIRE(all_bits.count() == 10240);
    }

    {
      VolBox vb2(vb);

      shape_defs::BitVector all_bits(10240);
      vb2.set_bits_for_one_sphere(p, all_bits, 0);
      REQUIRE(all_bits.count() == 10240);
    }
  }

  SECTION("Set bits for an empty sphere") {
    Point3DList empty;
    VolBox vb_empty(empty, 1.0);

    for (float x = -10.0; x != 10.0; x += 1.0) {
      shape_defs::BitVector matches(0);
      Sphere p{x, x, x, 22.0};
      // What about proving that this does not clear any bits?
      // Ah, never mind.
      vb_empty.set_bits_for_one_sphere(p, matches, 0);
      REQUIRE(matches.empty());
    }
  }

  SECTION("Set bits for one non-empty sphere") {
    const float r = 5.0;
    float r_sphere = get_max_extent(sphere);
    unsigned int total = 0;
    for (float x = -15.0; x != 15.0; x += 1.0) {
      shape_defs::BitVector vb_matches(sphere.size()),
          brute_force_matches(sphere.size());
      Sphere p{x, x, x, r};
      vb.set_bits_for_one_sphere(p, vb_matches, 0);
      get_bits(sphere, x, x, x, r, brute_force_matches);
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
    SphereList center_spheres;
    const float r = 5.0;

    shape_defs::BitVector brute_force(sphere.size());
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back({x, x, x, r});
      get_bits(sphere, x, x, x, r, brute_force);
    }

    shape_defs::BitVector vb_matches;
    vb.set_bits_for_spheres(center_spheres, vb_matches, true, 0);
    REQUIRE(brute_force == vb_matches);
    REQUIRE(vb_matches.count() > 0);
    REQUIRE(vb_matches.count() <= sphere.size());
  }

  SECTION("TODO Write tests for custom sphere scaling") {}

  SECTION("Get points within spheres") {
    SphereList center_spheres;
    const float r = 5.0;

    shape_defs::BitVector brute_force(sphere.size());
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back({x, x, x, r});
      get_bits(sphere, x, x, x, r, brute_force);
    }

    Point3DList bf_contained_points;
    get_contained_points(sphere, center_spheres, bf_contained_points);

    Point3DList contained_points;
    vb.get_points_within_spheres(center_spheres, contained_points, 0);
    REQUIRE(contained_points.size() > 0);
    REQUIRE(contained_points.size() == brute_force.count());

    // Not sure about the correctness of this test...
    REQUIRE(contained_points == bf_contained_points);
  }

  SECTION("Test VolBox containment for spheres of varying sizes.") {
    unsigned int total_points = sphere.size();
    const float R = get_max_extent(sphere);

    float d_r = (R - 1.5) / 10.0;
    float r = 1.5;
    while (r <= R) {
      const SphereList centers{{0, 0, 0, r}};

      Point3DList expected;
      get_contained_points(sphere, centers, expected);

      Point3DList contained;
      vb.get_points_within_spheres(centers, contained, 0);

      REQUIRE(get_max_extent(contained) <= r);
      REQUIRE(expected == contained);

      double expected_count = total_points * (r * r * r) / (R * R * R);
      REQUIRE_THAT(expected_count,
                   Catch::Matchers::WithinRel(contained.size(), 0.0475));

      r += d_r;
    }
  }

  SECTION("Test spherules at various locations within a VolBox") {
    unsigned int total_points = sphere.size();
    const float R = get_max_extent(sphere);
    const float r = 1.77; // Akin to carbon
    // How many cloud points to expect, based on density:
    const int exp_cnt = (int)(0.5 + total_points * (r * r * r) / (R * R * R));

    // Test various, fully-contained locations.
    const float max_offset = 0.9 * (R - r);
    const size_t steps = 10;
    const float d_center = max_offset / steps;
    float center = -max_offset;
    for (size_t i = 0; i < steps; ++i) {
      const SphereList centers{{center, 0, 0, r}};

      Point3DList expected;
      get_contained_points(sphere, centers, expected);

      Point3DList contained;
      vb.get_points_within_spheres(centers, contained, 0);

      float exp_max_extent = r + ::fabs(center);
      float act_max_extent = get_max_extent(contained);
      if (act_max_extent > exp_max_extent) {
        FAIL(format(R"LINES(  Expected max extent: {}
           Actual max: {})LINES",
                    exp_max_extent, act_max_extent));
      }
      REQUIRE(expected == contained);
      REQUIRE_THAT(exp_cnt,
                   Catch::Matchers::WithinRel(contained.size(), 0.075));

      center += d_center;
    }

    SECTION("No double counting of overlapping spherules") {
      // This is a simple test which demonstrates that,
      // if two spherules overlap, their corresponding cloud points
      // will not be double-counted.

      const Sphere atom({0, 0, 0, 1.7});

      const SphereList mol1{atom};
      // Duplicates, superposed!
      const SphereList mol2{atom, atom};

      Point3DList sphere;
      read_default_point_cloud(sphere);
      VolBox vb(sphere, 1.0);

      Point3DList contained1, contained2;
      vb.get_points_within_spheres(mol1, contained1, 0);
      vb.get_points_within_spheres(mol2, contained2, 0);
      REQUIRE(contained1 == contained2);
      REQUIRE(!contained1.empty());
    }
  }

  SECTION("Test setting VolBox bits with a fixed offset") {
    // TODO:  test get_points_within_spheres with a non-zero offset,
    // and with from_scratch = false.
    const shape::SphereList mol1{{0, 0, 0, 1.7}};

    const unsigned int num_cloud_points(vb.size());
    const unsigned int offset(10);
    shape_defs::BitVector bits1, bits2;
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
    shape::SphereList center_spheres;
    const float r = 5.0;
    for (int ix = -15; ix <= 15; ++ix) {
      float x(ix);
      center_spheres.push_back({x, x, x, r});
    }

    shape_defs::BitVector full_fp, unfolded;
    vb.set_bits_for_spheres(center_spheres, full_fp, true, 0);

    const unsigned int max_folds = 5;
    unsigned int num_folds;
    unsigned int folded_size = vb.size();
    for (num_folds = 0; num_folds != max_folds; ++num_folds, folded_size /= 2) {
      shape_defs::BitVector folded;
      folded.resize(folded_size);
      folded.reset();
      vb.set_folded_bits_for_spheres(center_spheres, folded, num_folds, 0);
      REQUIRE(folded.count() > 0);

      // cout << "Folds: " << num_folds << " = " << folded << endl;

      shape_defs::BitVector folded_brute;
      get_brute_force_folded(full_fp, folded_brute, num_folds);
      REQUIRE(folded == folded_brute);
    }
  }
}
} // namespace
} // namespace mesaac::shape
