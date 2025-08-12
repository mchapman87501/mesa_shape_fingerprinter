// Unit test for fingerprinter.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/catch_test_macros.hpp>

#include "mesaac_shape/fingerprinter.hpp"
#include "mesaac_shape/hammersley.hpp"

using namespace std;

namespace mesaac::shape {
namespace {
struct BoundingCube {
  float xmin, ymin, zmin, xmax, ymax, zmax;

  BoundingCube() { xmin = ymin = zmin = xmax = ymax = zmax = 0.0; }
};

struct TestFixture {
  void get_av_bounds(mol::AtomVector &atoms, BoundingCube &b) {
    bool first = true;
    for (const auto atom : atoms) {
      const float r(atom.radius());
      const auto &pos(atom.pos());
      float x(pos.x()), y(pos.y()), z(pos.z());

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
  } // namespace mesaac

  void test_for_atom_vector(mol::AtomVector &atoms, bool shouldBeEqual) {
    BoundingCube bc;
    get_av_bounds(atoms, bc);
    const unsigned int num_points(10240);
    PointList hamms;
    Hammersley::get_cuboid({.num_points = num_points,
                            .xmin = bc.xmin,
                            .xmax = bc.xmax,
                            .ymin = bc.ymin,
                            .ymax = bc.ymax,
                            .zmin = bc.zmin,
                            .zmax = bc.zmax},
                           hamms);
    VolBox vb(hamms, 1.0);

    Fingerprinter fp(vb);
    ShapeFingerprint sfp;
    fp.compute(atoms, sfp);
    REQUIRE((size_t)4 == sfp.size());
    for (int j = 0; j != (int)sfp.size(); ++j) {
      Fingerprint &fp(sfp[j]);
      REQUIRE((size_t)num_points == fp.size());
      //   cout << fp << endl;
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

TEST_CASE("mesaac::shape::Fingerprinter", "[mesaac]") {
  TestFixture fixture;

  SECTION("Test Symmetric Atom Vector") {
    // These atoms are regularly spaced in a straight line along x.
    // Their flips should all produce identical fingerprints.
    mol::AtomVector atoms;
    for (float i = -4.0; i <= 4.0; i += 2.0) {
      mol::Atom a({.atomic_num = 12, .pos = {i, 0, 0}});
      atoms.push_back(a);
    }
    fixture.test_for_atom_vector(atoms, true);
  }

  SECTION("Test Asymmetric Atom Vector") {
    // These atoms are regularly spaced in a straight line along x.
    // They 'wobble' in y.  Their flips should produce different fps.
    mol::AtomVector atoms;
    float y = 0.5;
    for (float i = -4.0; i <= 4.0; i += 2.0) {
      mol::Atom a({.atomic_num = 12, .pos = {i, y, 0}});
      atoms.push_back(a);

      y = -y;
    }
    fixture.test_for_atom_vector(atoms, false);
  }
}

} // namespace
} // namespace mesaac::shape
