// Unit test for license_info_gen.
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

#include "license_info_gen.h"

using namespace std;

namespace mesaac 
{
    class WB: public LicenseInfoGen {
    public:
        WB(int expYear, int expMonth, MesaacFeatures& features,
           string ownerName):
           LicenseInfoGen(expYear, expMonth, features, ownerName)
        {}
        
        string rawString() {
            return rawString_();
        }
        
        string digest() {
            return digest_();
        }
        
        string encrypted(string src) {
            return privateEncrypt_(src);
        }
        
        string decrypted(RSA *key, string src) {
            return publicDecrypt_(key, src);
        }
    };
    
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testBasic);
        CPPUNIT_TEST(testToLicenseStr);
        // CPPUNIT_TEST(testCrypting);
        CPPUNIT_TEST(testValidated);
        CPPUNIT_TEST(testExpiration);
        
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }
        
        void testBasic() {
            mesaac::MesaacFeatures features;
            LicenseInfoGen li(2008, 10, features, "John Don Juan");
        }
        
        void testToLicenseStr() {
            mesaac::MesaacFeatures features;
            features.set(MFE_SAESAR);
            features.set(MFE_GroupingModule);
            LicenseInfoGen li(2008, 10, features, "John Don Juan");
            string licenseStr(li.toLicenseStr());
            CPPUNIT_ASSERT_EQUAL((size_t)67, licenseStr.size());
        }

        // // Disabled because the public key is no longer directly accessible.
        // void testCrypting() {
        //     mesaac::MesaacFeatures features;
        //     WB lg(2008, 10, features, "FOO");
        //     WBLM lm;
        //     RSA *pubKey = lm.getKey();
        //     string orig("Testing");
        //     string encrypted(lg.encrypted(orig));
        //     string decrypted(lg.decrypted(pubKey, encrypted));
        //     CPPUNIT_ASSERT_EQUAL(orig, decrypted);
        // }

        void testValidated() {
            mesaac::MesaacFeatures features;
            features.set(MFE_SAESAR);
            features.set(MFE_GroupingModule);
            int year = 2008;
            int month = 10;
            string owner("Somebody c/o Some Co.");
            LicenseInfoGen li(year, month, features, owner);
            string licenseStr(li.toLicenseStr());
            LicenseInfo *lmi = LicenseInfo::fromLicenseStr(licenseStr, owner);
            CPPUNIT_ASSERT(0 != lmi);
        }
        
        void testExpiration() {
            mesaac::MesaacFeatures features;
            features.set(MFE_SAESAR);
            features.set(MFE_GroupingModule);
            string owner("Somebody c/o Some Co.");

            const time_t nowT = time(0);
            struct tm *now = localtime(&nowT);
            // LicenseInfo expects year in Current Era (CE),
            // month as 1..12 inclusive.  Contrast with values
            // returned by localtime: year relative to 1900 CE,
            // month in 0..11
            int year = now->tm_year + 1900;
            int month = now->tm_mon + 1;
            
            {
                LicenseInfoGen li(year, month, features, owner);
                cout << "Validating license string " << li.toLicenseStr() << endl;
                LicenseInfo *info = LicenseInfo::fromLicenseStr(
                    li.toLicenseStr(), owner);
                CPPUNIT_ASSERT(info != 0);
                CPPUNIT_ASSERT(!info->isExpired());
            }

            {
                LicenseInfoGen li(year - 1, 1, features, owner);
                LicenseInfo *info = LicenseInfo::fromLicenseStr(
                    li.toLicenseStr(), owner);
                CPPUNIT_ASSERT(info != 0);
                CPPUNIT_ASSERT(info->isExpired());
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
