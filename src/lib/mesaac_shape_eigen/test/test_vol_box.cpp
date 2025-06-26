// Unit test for vol_box
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <cmath>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "mesaac_shape/vol_box.h"

using namespace std;

namespace mesaac {
using namespace shape_eigen;

class TestCase : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestCase);
  CPPUNIT_TEST(test_copying);
  CPPUNIT_TEST(test_set_bits_for_one_sphere_empty);
  CPPUNIT_TEST(test_set_bits_for_one_sphere);
  CPPUNIT_TEST(test_set_bits_for_spheres);
  CPPUNIT_TEST(test_custom_scale);
  CPPUNIT_TEST(test_get_points_within_spheres);
  CPPUNIT_TEST(test_growing_spherule);
  CPPUNIT_TEST(test_moving_spherule);
  CPPUNIT_TEST(test_overlapping_spherules);
  CPPUNIT_TEST(test_nonzero_offset);

  CPPUNIT_TEST_SUITE_END();

protected:
  bool almost_equal(float expected, float actual, float max_fract = 1.0e-6,
                    bool verbose = false) {
    float err = ::fabs(expected - actual);
    float fract_err = err / ::fabs(expected);
    bool result = true;
    if (expected == 0) {
      result = (err <= max_fract);
    } else {
      result = (fract_err <= max_fract);
    }
    if (verbose || !result) {
      cout << "almost_equal(" << expected << ", " << actual
           << ", fract=" << max_fract << ")"
           << endl
           // << "        Actual err: " << err << endl
           << "    Fractional err: " << fract_err << endl;
      if (!result) {
        cout << "    FAILED!" << endl;
      }
    }
    return result;
  }

  void read_test_points(string pathname, PointList &points) {
    pathname = string("../../../../test_data/hammersley/") + pathname;
    points.clear();
    ifstream inf(pathname.c_str());
    if (!inf) {
      ostringstream msg;
      msg << "Could not open " << pathname << " for reading." << endl;
      CPPUNIT_FAIL(msg.str());
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

  VolBox *new_volbox() {
    PointList sphere;

    read_default_sphere(sphere);
    VolBox *result = new VolBox(sphere, 1.0);
    return result;
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

public:
  void test_copying() {
    auto_ptr<VolBox> vb(new_volbox());
    Point p(make_point(0, 0, 0, 22.0));

    {
      BitVector all_bits(10240);
      vb->set_bits_for_one_sphere(p, all_bits, 0);
      CPPUNIT_ASSERT_EQUAL((size_t)10240, all_bits.count());
    }

    {
      VolBox vb2(*vb);

      BitVector all_bits(10240);
      vb2.set_bits_for_one_sphere(p, all_bits, 0);
      CPPUNIT_ASSERT_EQUAL((size_t)10240, all_bits.count());
    }
  }

  void test_set_bits_for_one_sphere_empty() {
    PointList empty;
    VolBox vb(empty, 1.0);

    for (float x = -10.0; x != 10.0; x += 1.0) {
      BitVector matches(0);
      Point p(make_point(x, x, x, 22.0));
      // What about proving that this does not clear any bits?
      // Ah, never mind.
      vb.set_bits_for_one_sphere(p, matches, 0);
      CPPUNIT_ASSERT_EQUAL((size_t)0, matches.count());
    }
  }

  void test_set_bits_for_one_sphere() {
    PointList sphere;
    read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    const float r = 5.0;
    float r_sphere = get_max_extent(sphere);
    unsigned int total = 0;
    for (float x = -15.0; x != 15.0; x += 1.0) {
      BitVector vb_matches(sphere.size()), brute_force_matches(sphere.size());
      Point p(make_point(x, x, x, r));
      vb.set_bits_for_one_sphere(p, vb_matches, 0);
      get_bits(sphere, x, x, x, r, brute_force_matches);
      CPPUNIT_ASSERT_EQUAL(brute_force_matches, vb_matches);
      if (::fabs(x) > r_sphere) {
        CPPUNIT_ASSERT(vb_matches.count() == 0);
      }
      total += vb_matches.count();
    }
    // Try to ensure that the tests matched at least some points.
    CPPUNIT_ASSERT(total > 0);
  }

  void test_set_bits_for_spheres() {
    PointList sphere;
    read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    PointList center_spheres;
    const float r = 5.0;

    BitVector brute_force(sphere.size());
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back(make_point(x, x, x, r));
      get_bits(sphere, x, x, x, r, brute_force);
    }

    BitVector vb_matches;
    vb.set_bits_for_spheres(center_spheres, vb_matches, true, 0);
    CPPUNIT_ASSERT_EQUAL(brute_force, vb_matches);
    CPPUNIT_ASSERT(vb_matches.count() > 0);
    CPPUNIT_ASSERT(vb_matches.count() <= sphere.size());
  }

  void test_custom_scale() {
    cout << "TODO:  Write tests for custom sphere scaling" << endl;
  }

  void test_get_points_within_spheres() {
    PointList sphere;
    read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    PointList center_spheres;
    const float r = 5.0;

    BitVector brute_force(sphere.size());
    for (float x = -15.0; x != 15.0; x += 1.0) {
      center_spheres.push_back(make_point(x, x, x, r));
      get_bits(sphere, x, x, x, r, brute_force);
    }

    PointList bf_contained_points;
    get_contained_points(sphere, center_spheres, bf_contained_points);

    PointList contained_points;
    vb.get_points_within_spheres(center_spheres, contained_points, 0);
    CPPUNIT_ASSERT(contained_points.size() > 0);
    CPPUNIT_ASSERT(contained_points.size() == brute_force.count());

    // Not sure about the correctness of this test...
    CPPUNIT_ASSERT(contained_points == bf_contained_points);
  }

  void test_growing_spherule() {
    PointList sphere;
    read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);
    unsigned int total_points = sphere.size();
    const float R = get_max_extent(sphere);

    float d_r = (R - 1.5) / 10.0;
    for (float r = 1.5; r <= R; r += d_r) {
      PointList centers;
      centers.push_back(make_point(0, 0, 0, r));

      PointList expected;
      get_contained_points(sphere, centers, expected);

      PointList contained;
      vb.get_points_within_spheres(centers, contained, 0);

      CPPUNIT_ASSERT(get_max_extent(contained) <= r);
      CPPUNIT_ASSERT(expected == contained);

      float expected_count = total_points * (r * r * r) / (R * R * R);
      CPPUNIT_ASSERT(almost_equal(expected_count, contained.size(), 0.05));
    }
  }

  void test_moving_spherule() {
    PointList sphere;
    read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);
    unsigned int total_points = sphere.size();
    const float R = get_max_extent(sphere);
    const float r = 1.77; // Akin to carbon
    // How many cloud points to expect, based on density:
    const int exp_cnt = (int)(0.5 + total_points * (r * r * r) / (R * R * R));

    // Test various, fully-contained locations.
    const float max_offset = 0.9 * (R - r);
    const float d_center = max_offset / 10.0;
    for (float center = -max_offset; center <= max_offset; center += d_center) {
      PointList centers;
      centers.push_back(make_point(center, 0, 0, r));

      PointList expected;
      get_contained_points(sphere, centers, expected);

      PointList contained;
      vb.get_points_within_spheres(centers, contained, 0);

      float exp_max_extent = r + ::fabs(center);
      float act_max_extent = get_max_extent(contained);
      if (act_max_extent > exp_max_extent) {
        ostringstream msg;
        msg << "  Expected max extent: " << exp_max_extent << endl
            << "          Actual max: " << act_max_extent << endl;
        CPPUNIT_FAIL(msg.str());
      }
      CPPUNIT_ASSERT(expected == contained);

      CPPUNIT_ASSERT(almost_equal(exp_cnt, contained.size(), 0.075));
    }
  }

  void test_overlapping_spherules() {
    // This is a simple test which demonstrates that,
    // if two spherules overlap, their corresponding cloud points
    // will not be double-counted.

    const Point atom(make_point(0, 0, 0, 1.7));

    PointList mol1, mol2;
    mol1.push_back(atom);
    // Duplicates, superposed!
    mol2.push_back(atom);
    mol2.push_back(atom);

    PointList sphere;
    read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    PointList contained1, contained2;
    vb.get_points_within_spheres(mol1, contained1, 0);
    vb.get_points_within_spheres(mol2, contained2, 0);
    CPPUNIT_ASSERT(contained1 == contained2);
    CPPUNIT_ASSERT(contained1.size() > 0);
  }

  void test_nonzero_offset() {
    // TODO:  test get_points_within_spheres with a non-zero offset,
    // and with from_scratch = false.
    const Point atom(make_point(0, 0, 0, 1.7));

    PointList mol1;
    mol1.push_back(atom);

    PointList sphere;
    read_default_sphere(sphere);
    VolBox vb(sphere, 1.0);

    const unsigned int num_cloud_points(vb.size());
    const unsigned int offset(10);
    BitVector bits1, bits2;
    bits1.resize(num_cloud_points);
    bits2.resize(num_cloud_points + offset);

    vb.set_bits_for_spheres(mol1, bits1, false, 0);
    vb.set_bits_for_spheres(mol1, bits2, false, offset);

    CPPUNIT_ASSERT(0 < num_cloud_points);

    unsigned int i;
    for (i = 0; i != num_cloud_points; i++) {
      CPPUNIT_ASSERT_EQUAL(bits1.test(i), bits2.test(i + offset));
    }
    CPPUNIT_ASSERT_EQUAL(bits1.count(), bits2.count());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(TestCase);
}; // namespace mesaac

int main(int, char **) {
  int result = 0;
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry =
      CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest(registry.makeTest());

  if (!runner.run()) {
    result = 1;
  }
  return result;
}
