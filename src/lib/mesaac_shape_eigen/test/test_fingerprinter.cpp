// Unit test for fingerprinter.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>

#include "mesaac_shape/fingerprinter.h"
#include "mesaac_shape/hammersley.h"

using namespace std;
using namespace mesaac::shape_eigen;
using namespace mesaac::mol;

namespace mesaac {
class BoundingCube {
public:
  float xmin, ymin, zmin, xmax, ymax, zmax;

  BoundingCube() { xmin = ymin = zmin = xmax = ymax = zmax = 0.0; }
};

class TestCase : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(TestCase);
  CPPUNIT_TEST(testSymmetricAtomVector);
  CPPUNIT_TEST(testAsymmetricAtomVector);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp() {}

  void tearDown() {}

  void testSymmetricAtomVector() {
    // These atoms are regularly spaced in a straight line along x.
    // Their flips should all produce identical fingerprints.
    AtomVector atoms;
    for (int i = -4.0; i <= 4.0; i += 1.7) {
      Atom *a = new Atom();
      a->x(i);
      a->y(0.0);
      a->z(0.0);
      a->atomic_num(12);
      atoms.push_back(a);
    }
    test_for_atom_vector(atoms, true);
  }

  void testAsymmetricAtomVector() {
    // These atoms are regularly spaced in a straight line along x.
    // They 'wobble' in y.  Their flips should produce different fps.
    AtomVector atoms;
    float y = 0.5;
    for (int i = -4.0; i <= 4.0; i += 1.7) {
      Atom *a = new Atom();
      a->x(i);
      a->y(y);
      a->z(0.0);
      a->atomic_num(12);
      atoms.push_back(a);

      y = -y;
    }
    test_for_atom_vector(atoms, false);
  }

protected:
  void get_av_bounds(AtomVector &atoms, BoundingCube &b) {
    AtomVector::iterator i;
    bool first = true;
    for (i = atoms.begin(); i != atoms.end(); ++i) {
      Atom *a(*i);
      float r(a->radius());
      float x(a->x()), y(a->y()), z(a->z());

      if (first) {
        b.xmin = x - r;
        b.xmax = x + r;
        b.ymin = y - r;
        b.ymax = y + r;
        b.zmin = z - r;
        b.zmax = z + r;
        first = false;
      } else {
        b.xmin = (b.xmin < x - r) ? b.xmin : x - r;
        b.xmax = (b.xmax > x + r) ? b.xmax : x + r;
        b.ymin = (b.ymin < y - r) ? b.ymin : y - r;
        b.ymax = (b.ymax > y + r) ? b.ymax : y + r;
        b.zmin = (b.zmin < z - r) ? b.zmin : z - r;
        b.zmax = (b.zmax > z + r) ? b.zmax : z + r;
      }
    }
  }

  void test_for_atom_vector(AtomVector &atoms, bool shouldBeEqual) {
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
    CPPUNIT_ASSERT_EQUAL((size_t)4, sfp.size());
    for (int j = 0; j != (int)sfp.size(); ++j) {
      Fingerprint &fp(sfp[j]);
      CPPUNIT_ASSERT_EQUAL((size_t)num_points, fp.size());
      cout << fp << endl;
      if (j > 0) {
        Fingerprint &prev(sfp[j - 1]);
        if (shouldBeEqual) {
          float either = (fp | prev).count();
          float both = (fp & prev).count();
          // Allow for small sampling errors.
          CPPUNIT_ASSERT((both / either) >= 0.93);
        } else {
          CPPUNIT_ASSERT(fp != prev);
        }
      }
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
