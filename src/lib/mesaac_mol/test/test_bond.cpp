// Unit test for mol::Bond
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "mesaac_mol/bond.h"

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
            mol::Bond b;
            b.a0(1);
            b.a1(2);
            CPPUNIT_ASSERT_EQUAL(1u, b.a0());
            CPPUNIT_ASSERT_EQUAL(2u, b.a1());
            
            CPPUNIT_ASSERT_EQUAL(mol::Bond::BTE_SINGLE, b.type());
            b.type(mol::Bond::BTE_DOUBLE);
            CPPUNIT_ASSERT_EQUAL(mol::Bond::BTE_DOUBLE, b.type());
            
            b.stereo(mol::Bond::BSE_NOT_STEREO);
            CPPUNIT_ASSERT_EQUAL(mol::Bond::BSE_NOT_STEREO, b.stereo());
            
            b.optional_cols("xxxrrrccc");
            CPPUNIT_ASSERT_EQUAL(string("xxxrrrccc"), b.optional_cols());
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
