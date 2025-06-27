// Unit test for license_info.
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

#include "license_info.h"

using namespace std;

namespace mesaac 
{
    class WB: public LicenseInfo {
    public:
        WB(int expYear, int expMonth, MesaacFeatures& features,
           string ownerName, string signature):
           LicenseInfo(expYear, expMonth, features, ownerName, signature)
        {}
        
        string rawString() {
            return rawString_();
        }
        
        string digest() {
            return digest_();
        }
    };
    
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testBasic);
        CPPUNIT_TEST(testRawString);
        CPPUNIT_TEST(testDigest);
        CPPUNIT_TEST(testIsExpired);
        CPPUNIT_TEST(testExpirationDateStr);
        CPPUNIT_TEST(testHasFeatures);
        CPPUNIT_TEST(testMissingFeatures);
        CPPUNIT_TEST(testPathname);
        CPPUNIT_TEST(testFromInvalidLicenseStr);
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }
        
        void testBasic() {
            mesaac::MesaacFeatures features;
            string signature("No signature");
            LicenseInfo li(2008, 10, features, "John Don Juan", signature);
        }
        
        void testRawString() {
            mesaac::MesaacFeatures features;
            string signature("No signature");
            WB li(2008, 10, features, "John Don Juan", signature);
            string expected("2008\t10\t000000\tJohn Don Juan");
            CPPUNIT_ASSERT_EQUAL(li.rawString(), expected);
        }

        void testDigest() {
            mesaac::MesaacFeatures features;
            string signature("No signature");
            WB li(2008, 10, features, "John Don Juan", signature);
            CPPUNIT_ASSERT_EQUAL((int)(li.digest().size()), 20);
        }

        void testIsExpired() {
            mesaac::MesaacFeatures features;
            string signature("No signature");
            const time_t nowT = time(0);
            struct tm *now = localtime(&nowT);
            // LicenseInfo expects year in Current Era (CE),
            // month as 1..12 inclusive.  Contrast with values
            // returned by localtime: year relative to 1900 CE,
            // month in 0..11
            int year = now->tm_year + 1900;
            int month = now->tm_mon + 1;
            {
                WB li(year, month, features, 
                      "John Don Juan", signature);
                CPPUNIT_ASSERT(!li.isExpired());
            }
            
            {
                month -= 1;
                if (month < 0) {
                    month += 12;
                    year -= 1;
                }
                WB li(year, month, features, "John Don Juan", signature);
                // Even if this is the 1st of the month, the license should
                // have expired (at midnight local time).
                CPPUNIT_ASSERT(li.isExpired());
            }
            
            // TODO:  Modify wb to let us fool LicenseInfo re the current
            // time.
            // The tests above can verify correct boundary behavior only
            // when this is exactly the end of the month.
        }
        
        void testExpirationDateStr() {
            mesaac::MesaacFeatures featureSet;
            string signature("no signature");
            WB li(2008, 10, featureSet, "Bubba", signature);
            
            string expDate(li.expirationDateStr());
            // The actual format of the expiration date string depends on
            // the current locale.
            // Which makes this a very weak test -- it duplicates much of
            // the logic in LicenseInfo.
            struct tm expectedTM;
            expectedTM.tm_sec = 0;
            expectedTM.tm_min = 0;
            expectedTM.tm_hour = 0;
            expectedTM.tm_mday = 1;
            expectedTM.tm_wday = 0;
            expectedTM.tm_yday = 0;
            expectedTM.tm_isdst = 0;
            expectedTM.tm_zone = 0;
            expectedTM.tm_gmtoff = 0;
            expectedTM.tm_year = 2008 - 1900;
            expectedTM.tm_mon = 10;

            time_t expectedT = mktime(&expectedTM);
            string expected(ctime(&expectedT));
            CPPUNIT_ASSERT_EQUAL(expected, expDate);
        }
        
        void testHasFeatures() {
            MesaacFeatures featureSet;
            string signature("no signature");
            WB li1(2008, 10, featureSet, "Bubba", signature);
            
            MesaacFeatures requiredSet;
            requiredSet.set(MFE_SAESAR);
            requiredSet.set(MFE_GroupingModule);
            
            CPPUNIT_ASSERT(!li1.hasFeatures(requiredSet));
            
            WB li2(2008, 10, requiredSet, "Bubba2", signature);
            CPPUNIT_ASSERT(li2.hasFeatures(requiredSet));
            
            requiredSet.reset(MFE_SAESAR);
            CPPUNIT_ASSERT(li2.hasFeatures(requiredSet));
        }
        
        void testMissingFeatures() {
            MesaacFeatures featureSet;
            string signature("no signature");
            WB li1(2008, 10, featureSet, "Bubba", signature);
            
            MesaacFeatures requiredSet;
            requiredSet.set(MFE_SAESAR);
            requiredSet.set(MFE_GroupingModule);
            
            CPPUNIT_ASSERT(li1.missingFeatures(requiredSet) == requiredSet);
            
            WB li2(2008, 10, requiredSet, "Bubba2", signature);
            MesaacFeatures empty;
            CPPUNIT_ASSERT(li2.missingFeatures(requiredSet) == empty);
            
            requiredSet.reset(MFE_SAESAR);
            CPPUNIT_ASSERT(li2.missingFeatures(requiredSet) == empty);
            
            WB li3(2008, 10, requiredSet, "Bubba3", signature);
            requiredSet.set(MFE_SAESAR);
            MesaacFeatures saesar;
            saesar.set(MFE_SAESAR);
            CPPUNIT_ASSERT(li3.missingFeatures(requiredSet) == saesar);
            CPPUNIT_ASSERT(li3.missingFeatures(requiredSet) != empty);
        }
        
        void testPathname() {
            MesaacFeatures featureSet;
            string signature("no signature");
            WB li(2008, 10, featureSet, "Bubba", signature);
            
            CPPUNIT_ASSERT(li.pathname() == "");
            li.setPathname("FOO.TXT");
            CPPUNIT_ASSERT(li.pathname() == "FOO.TXT");
        }
        
        void testFromInvalidLicenseStr() {
            // This test is just to make sure that fromLicenseStr does not
            // seg-fault on an invalid license string.
            string lStr = "Invalid license string";
            string owner = "Nemo";
            LicenseInfo *li = LicenseInfo::fromLicenseStr(lStr, owner);
            CPPUNIT_ASSERT(!li->isValid());
            lStr = "008-10-101-PUREGARBAGE";
            li = LicenseInfo::fromLicenseStr(lStr, owner);
            CPPUNIT_ASSERT(!li->isValid());
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
