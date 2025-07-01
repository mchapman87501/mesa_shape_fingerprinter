// Unit test for fingerprinter.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <iostream>

#include "mesaac_shape_eigen/fingerprinter.hpp"
#include "mesaac_shape_eigen/hammersley.hpp"

using namespace std;

namespace mesaac::shape_eigen {

namespace {
struct BoundingCube {
  float xmin, ymin, zmin, xmax, ymax, zmax;

  BoundingCube() { xmin = ymin = zmin = xmax = ymax = zmax = 0.0; }
};

struct TestFixture {
  void get_av_bounds(mol::AtomVector &atoms, BoundingCube &b) {
    bool first = true;
    for (auto &atom : atoms) {
      float r(atom.radius());
      float x(atom.x()), y(atom.y()), z(atom.z());

      if (first) {
        b.xmin = x - r;
        b.xmax = x + r;
        b.ymin = y - r;
        b.ymax = y + r;
        b.zmin = z - r;
        b.zmax = z + r;
        first = false;
      } else {
        b.xmin = min(b.xmin, x - r);
        b.xmax = max(b.xmax, x + r);
        b.ymin = min(b.ymin, y - r);
        b.ymax = max(b.ymax, y + r);
        b.zmin = min(b.zmin, z - r);
        b.zmax = max(b.zmax, z + r);
      }
    }
  }

  void test_for_atom_vector(mol::AtomVector &atoms, bool shouldBeEqual) {
    BoundingCube bc;
    get_av_bounds(atoms, bc);
    const unsigned int num_points(10240);
    PointList hamms;
    Hammersley::get_cubic(bc.xmin, bc.xmax, bc.ymin, bc.ymax, bc.zmin, bc.zmax,
                          num_points, hamms);
    VolBox vb(hamms, 1.0);

    Fingerprinter fp(vb);
    ShapeFingerprint sfp;
    fp.compute(atoms, sfp);
    REQUIRE(sfp.size() == 4);
    for (int j = 0; j != (int)sfp.size(); ++j) {
      Fingerprint &fp(sfp[j]);
      REQUIRE(fp.size() == num_points);
      cout << fp << endl;
      if (j > 0) {
        Fingerprint &prev(sfp[j - 1]);
        if (shouldBeEqual) {
          float either = (fp | prev).count();
          float both = (fp & prev).count();
          // Allow for small sampling errors.
          REQUIRE((both / either) >= 0.93);
        } else {
          REQUIRE(fp != prev);
        }
      }
    }
  }
};

TEST_CASE("mesaac::shape_eigen::Fingerprinter", "[mesaac]") {
  TestFixture fixture;

  SECTION("Test Symmetric Atom Vector") {
    // These atoms are regularly spaced in a straight line along x.
    // Their flips should all produce identical fingerprints.
    mol::AtomVector atoms;
    for (float i = -4.0; i <= 4.0; i += 2.0) {
      mol::Atom atom({12, {float(i), 0, 0}});
      atoms.push_back(atom);
    }
    fixture.test_for_atom_vector(atoms, true);
  } // namespace shape_eigen

  SECTION("Test Asymmetric Atom Vector") {
    // These atoms are regularly spaced in a straight line along x.
    // They 'wobble' in y.  Their flips should produce different fps.
    mol::AtomVector atoms;
    float y = 0.5;
    for (float i = -4.0; i <= 4.0; i += 2.0) {
      mol::Atom atom({12, {float(i), y, 0}});
      atoms.push_back(atom);
      y = -y;
    }
    fixture.test_for_atom_vector(atoms, false);
  }
}

} // namespace
} // namespace mesaac::shape_eigen
