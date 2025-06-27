// Unit test for b64
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

#include "mesaac_common/b64.h"

using namespace std;

namespace mesaac 
{
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testBasic);
        CPPUNIT_TEST(testFuzz);
        CPPUNIT_TEST(testAllBytes);
        CPPUNIT_TEST(testCorrupted);

        CPPUNIT_TEST_SUITE_END();

        void roundtrip(string orig) {
            B64 codec;
            
            string encoded = codec.encode(orig);
            // cout << "Encoded: " << encoded << endl;
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
        
        char randomChar() {
            // Get a random character which is not a valid B64 char.
            static string alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
            char result = '_';
            if ((random() & 0xFF) >= 0x10) {
                const int MAX = (int)('+') - 1;
                result = (char)(random() % MAX);
            } else {
                const int MIN = (int)('z') + 1;
                const int MAX = 255;
                result = (char)(MIN + (random() % (MAX - MIN)));
            }
            return result;
        }
        
        void testCorrupted() {
            // Fuzz test, decoding corrupted B64 strings.
            // In this case "corrupted" means only that the string
            // contains at least one invalid B64 character.
            B64 codec;
            const int LENGTH = 160;
            
            for (int i = 1; i < LENGTH; i++) {
                string src = "";
                for (int j = 0; j < i; j++) {
                    src += (char)(random() & 0xFF);
                }
            
                string encoded = codec.encode(src);
                // Append or replace a single character of the encoded
                // string w. an invalid B64 character.
                int byte_to_munge = (random() % i);
                encoded[byte_to_munge] = randomChar();
                CPPUNIT_ASSERT_THROW(codec.decode(encoded), std::invalid_argument);
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
