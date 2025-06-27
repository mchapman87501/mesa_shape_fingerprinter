// Unit test for b32
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

#include "mesaac_common/b32.h"

using namespace std;

namespace mesaac 
{
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testBasic);
        CPPUNIT_TEST(testFuzz);
        CPPUNIT_TEST(testAllBytes);
        
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }
        
        void roundtrip(string orig) {
            B32 codec;
            
            string encoded = codec.encode(orig);
            //cout << "Encoded " << encoded << endl;
            string decoded = codec.decode(encoded);
            CPPUNIT_ASSERT_EQUAL(orig, decoded);
            //cout << "Round-trip string of length " << orig.size() << endl;
        }

        void testBasic() {
            string orig;
            orig += (char)0xF0;
            orig += (char)0x0F;
            roundtrip(orig);
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
        
        void testAllBytes() {
            for (int i = 0; i < 256; i++) {
                string src = "";
                for (int j = 0; j < 256; j++) {
                    src += (char)((i + j) & 0xFF);
                }
                roundtrip(src);
            }
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
