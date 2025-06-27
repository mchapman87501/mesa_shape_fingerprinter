// Unit test for license_info_file.
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

#include "license_info_file.h"

using namespace std;

namespace mesaac 
{
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testGetPathname);
        
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }
        
        void testGetPathname() {
            string pathname = license_info_file::getPathname();
            // How to perform this test?  It depends heavily on the runtime
            // environment -- environment variable definitions, and license
            // files which you will probably have installed for other
            // purposes.
            // Would be nice to have Python around, to temporarily move
            // license files out of the way.
            
            // If there is no license file in the current directory, create
            // one, test again, and remove it.
            if (pathname == "") {
                ofstream outf(license_info_file::getBasename().c_str());
                outf << "Invalid owner" << endl
                     << "008-10-1-INVALIDLICENSESTR" << endl;
                outf.close();
                pathname = license_info_file::getPathname();
                unlink(license_info_file::getBasename().c_str());
            }
            cout << "License file pathname: " << pathname << endl;
            CPPUNIT_ASSERT(pathname != "");
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
