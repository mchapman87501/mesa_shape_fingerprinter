// Unit test for hammersley
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

#include "mesaac_shape/hammersley.h"

using namespace std;
using namespace mesaac::shape_eigen;

namespace mesaac {
class TestCase : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestCase);
  CPPUNIT_TEST(testBasic);
  CPPUNIT_TEST(testGetCubic);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testBasic() {
    Hammersley h;
    const int num_points = 10240;
    int num_generated = 0;

    h.start(num_points);
    Point p;
    while (h.next_point(p)) {
      CPPUNIT_ASSERT_EQUAL((size_t)3, p.size());
      CPPUNIT_ASSERT_EQUAL((num_generated + 1.0f) / num_points, p[0]);
      // Can't think of any succinct tests for subsequent coords.
      CPPUNIT_ASSERT(0 < p[1] <= 1.0);
      CPPUNIT_ASSERT(0 < p[2] <= 1.0);

      spot_check(p, num_generated, num_points);
      num_generated++;
    }
    CPPUNIT_ASSERT_EQUAL(num_points, num_generated);
  }

  void testGetCubic() {
    // Lacking better ideas, how about a semi-random test for a
    // flattish volume?
    PointList points;
    float xmin = -2.0, xmax = 3.1, ymin = 5.0, ymax = 10.1, zmin = 0.0,
          zmax = 1.8;
    unsigned int num_points = 10240;
    Hammersley::get_cubic(xmin, xmax, ymin, ymax, zmin, zmax, num_points,
                          points);

    CPPUNIT_ASSERT_EQUAL((size_t)num_points, points.size());
    PointList::iterator i;
    for (i = points.begin(); i != points.end(); ++i) {
      Point &p(*i);
      CPPUNIT_ASSERT(xmin <= p[0] && p[0] <= xmax);
      CPPUNIT_ASSERT(ymin <= p[1] && p[1] <= ymax);
      CPPUNIT_ASSERT(zmin <= p[2] && p[2] <= zmax);
    }
    // Ensure the points fill the volume.
    check_max_extents(points, xmin, xmax, ymin, ymax, zmin, zmax);
  }

protected:
  bool approx(float expected, float actual, float max_fract = 1.0e-6) {
    bool result = true;
    if (0 == expected) {
      result = (std::abs(expected - actual) <= max_fract);
    } else {
      result = ((std::abs(expected - actual) / expected) <= max_fract);
    }
    if (!result) {
      cout << "approx(" << expected << ", " << actual << ") = false" << endl;
    }
    return result;
  }

  void check_max_extents(const PointList &points, float xmin, float xmax,
                         float ymin, float ymax, float zmin, float zmax) {
    float axmin, axmax, aymin, aymax, azmin, azmax;
    if (points.size()) {
      axmin = axmax = points[0][0];
      aymin = aymax = points[0][1];
      azmin = azmax = points[0][2];
      PointList::const_iterator i;
      for (i = points.begin(); i != points.end(); ++i) {
        const Point &p(*i);
        float x(p[0]), y(p[1]), z(p[2]);
        axmin = (axmin < x) ? axmin : x;
        axmax = (axmax > x) ? axmax : x;
        aymin = (aymin < y) ? aymin : y;
        aymax = (aymax > y) ? aymax : y;
        azmin = (azmin < z) ? azmin : z;
        azmax = (azmax > z) ? azmax : z;
      }

      const float err = 1.0e-3;
      CPPUNIT_ASSERT(approx(xmin, axmin, err));
      CPPUNIT_ASSERT(approx(xmax, axmax, err));
      CPPUNIT_ASSERT(approx(ymin, aymin, err));
      CPPUNIT_ASSERT(approx(ymax, aymax, err));
      CPPUNIT_ASSERT(approx(zmin, azmin, err));
      CPPUNIT_ASSERT(approx(zmax, azmax, err));
    }
  }

  void check_point(Point &p, float x_exp, float y_exp, float z_exp) {
    CPPUNIT_ASSERT(approx(x_exp, p[0]));
    CPPUNIT_ASSERT(approx(y_exp, p[1]));
    CPPUNIT_ASSERT(approx(z_exp, p[2]));
  }

  void spot_check(Point &p, int index, int num_points) {
    // Spot-check a few points against expted values generated
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
