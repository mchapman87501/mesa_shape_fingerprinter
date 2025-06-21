// Unit test for mol::Atom
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "mesaac_mol/atom.h"
#include <string>

using namespace std;

namespace mesaac 
{
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testBasic);
        
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            
        }

        void tearDown() {
        }

        void testBasic() {
            mol::Atom a;
            a.atomic_num(6);
            CPPUNIT_ASSERT_EQUAL(6U, a.atomic_num());
            
            CPPUNIT_ASSERT_EQUAL(0.0f, a.x());
            CPPUNIT_ASSERT_EQUAL(0.0f, a.y());
            CPPUNIT_ASSERT_EQUAL(0.0f, a.z());
            
            CPPUNIT_ASSERT_EQUAL(string(""), a.optional_cols());
            CPPUNIT_ASSERT_EQUAL(string("C"), a.symbol());
            CPPUNIT_ASSERT_EQUAL(1.7f, a.radius());
            CPPUNIT_ASSERT(!a.is_hydrogen());
            
            a.atomic_num(1);
            a.optional_cols("fooo");
            CPPUNIT_ASSERT_EQUAL(string("H"), a.symbol());
            CPPUNIT_ASSERT_EQUAL(1.09f, a.radius());
            CPPUNIT_ASSERT(a.is_hydrogen());
            CPPUNIT_ASSERT_EQUAL(string("fooo"), a.optional_cols());
            
            a.atomic_num(8);
            CPPUNIT_ASSERT_EQUAL(string("O"), a.symbol());
            CPPUNIT_ASSERT(!a.is_hydrogen());
            CPPUNIT_ASSERT_EQUAL(string("fooo"), a.optional_cols());
            
            a.x(10.0);
            a.y(11.0);
            a.z(-200.5);
            CPPUNIT_ASSERT_EQUAL(10.0f, a.x());
            CPPUNIT_ASSERT_EQUAL(11.0f, a.y());
            CPPUNIT_ASSERT_EQUAL(-200.5f, a.z());
        }
        
    };

    CPPUNIT_TEST_SUITE_REGISTRATION(TestCase);
};

int
main(int, char **)
{
    int result = 0;
    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = 
        CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest(registry.makeTest());
    
    if (!runner.run())
    {
        result = 1;
    }
    return result;
}
