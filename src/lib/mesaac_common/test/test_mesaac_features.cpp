// Unit test for mesaac_features.
// Copyright (c) 2008 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

#include "mesaac_common/mesaac_features.h"

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
            MesaacFeatures f;
            CPPUNIT_ASSERT(features::toString(f) == "");
            f.set(MFE_GroupingModule);
            CPPUNIT_ASSERT(features::toString(f) == "grouping");
            f.set(MFE_SAESAR);
            CPPUNIT_ASSERT(features::toString(f) == "grouping, saesar");
            f.set(MFE_FingerprinterModule);
            CPPUNIT_ASSERT(features::toString(f) == "grouping, fingerprinter, saesar");
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
