// Unit test for gzip
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>

#include "mesaac_common/gzip.h"

using namespace std;

namespace mesaac 
{
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testRoundTrip);
        CPPUNIT_TEST(testFuzz);
        // CPPUNIT_TEST(testCorrupted);
        
        CPPUNIT_TEST_SUITE_END();

    protected:
        void roundtrip(string orig) {
            string compressed = gzip::compress(orig);
            string decompressed = gzip::decompress(compressed);
            CPPUNIT_ASSERT_EQUAL(orig, decompressed);
        }

    public:
        void testRoundTrip() {
            string original("Some test, huh.");
            roundtrip(original);
        }

        void testFuzz() {
            for (int i = 0; i < 160; i++) {
                string src = "";
                for (int j = 0; j < i; j++) {
                    src += (char)(random() & 0xFF);
                }
                roundtrip(src);
                CPPUNIT_ASSERT_EQUAL(src.size(), (size_t)i);
            }
        }
        
        // void testCorrupted() {
        //     // Too hard to test for corruption using random manglings.
        //     // If compressed data is corrupted, then sometimes you'll get
        //     // a runtime_error.  Sometimes you'll get no error, but an
        //     // inconsistent result.  Sometimes you'll get a correct result.
        // }
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
