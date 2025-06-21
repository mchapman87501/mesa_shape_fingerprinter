// Unit test for mol::SDWriter
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

#include "mesaac_mol/io.h"

using namespace std;

namespace mesaac 
{
    string strdiff_summary(const string& s1, const string& s2) {
        unsigned int i;
        unsigned int i_max = s1.size();
        if (s2.size() > i_max) {
            i_max = s2.size();
        }
        ostringstream outf;
        for (i = 0; i != i_max; i++) {
            if (i >= s1.size()) {
                outf << "<";
            } else if (i >= s2.size()) {
                outf << ">";
            } else if (s1[i] != s2[i]) {
                outf << "X";
            } else {
                outf << ".";
            }
        }
        return outf.str();
    }
        
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testDimensionality);
        CPPUNIT_TEST(testBasic);
        CPPUNIT_TEST(testMalformedEmptyTag);
        CPPUNIT_TEST(testTransformAndTagging);
        
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            
        }

        void tearDown() {
        }

        void testDimensionality() {
            // Based on a layman's reading of MDL ctfile spec:
            // Dimensionality of a molecule should be 3 if it contains
            // any non-zero z coordinates, 2 otherwise.
            mol::Mol m;
            mol::Atom a;
            a.atomic_num(1);
            m.add_atom(a);
            CPPUNIT_ASSERT_EQUAL(2u, m.dimensionality());
            a.x(1.0);
            m.add_atom(a);
            CPPUNIT_ASSERT_EQUAL(2u, m.dimensionality());
            
            a.y(1.0);
            m.add_atom(a);
            CPPUNIT_ASSERT_EQUAL(2u, m.dimensionality());

            a.z(1.0);
            m.add_atom(a);
            CPPUNIT_ASSERT_EQUAL(3u, m.dimensionality());
        }
        
        void testBasic() {
            // OBS:  mol::SDWriter will write out its tags in by lexical order
            // of tag header.  This may not be the same as the input.
            // For simplified testing, use an input file whose tags are
            // already sorted lexically by tag header.
            string pathname("data/in/sorted_tags.sdf");
            ifstream inf(pathname.c_str());
            mol::SDReader reader(inf, pathname);
            
            ostringstream outs;
            mol::SDWriter writer(outs);
            
            mol::Mol m;
            while(reader.read(m)) {
                writer.write(m);
            }
            inf.close();
            
            // Output should be identical to input, except for the program data
            // lines.  Ignore the program data lines.
            istringstream actualf(outs.str());
            ifstream expectedf(pathname.c_str());
            unsigned int 
                i_mol_line = 0,
                i = 0;
            ostringstream diffs;
            while (actualf && expectedf) {
                i++;
                i_mol_line++;

                string expected, actual;
                std::getline(actualf, actual);
                std::getline(expectedf, expected);
                if (i_mol_line != 2) {
                    if (actual != expected) {
                        diffs << "Line " << i << ": '" << expected << "' != '" << actual << "'" << endl;
                    }
                }
                if (actual == "$$$$") {
                    i_mol_line = 0;
                }
            }
            if (actualf) {
                diffs << "Actual output has more lines than expected." << endl;
            } else if (expectedf) {
                diffs << "Expected output has more lines than actual." << endl;
            }
            // This is a good way to test for success, but a lousy way
            // to report differences...
            if (diffs.str().size() > 0) {
                ofstream actf("actual_test_sdwriter.sdf");
                actf << outs.str();
                actf.close();
                
                CPPUNIT_FAIL(diffs.str());
            }
        }
        
        void testMalformedEmptyTag() {
            // Some utilities produce invalid SD tags -- ones which have
            // a tag followed by just one empty line instead of two or more.
            // Ensure the writer can output these records correctly.
            string pathname("data/in/malformed_empty_tag_value.sdf");
            ifstream inf(pathname.c_str());
            mol::SDReader reader(inf, pathname);
            mol::Mol m;
            int occurrences = 0;
            while (reader.read(m)) {
                ostringstream outs;
                mol::SDWriter writer(outs);
                writer.write(m);
                
                string s(outs.str());
                ostringstream tag;
                tag << ">  <empty>" << endl << endl;
                if (string::npos != s.find(tag.str())) {
                    // If the tag is present, verify that its empty
                    // value is also present -- and fixed so it has an
                    // extra blank line.
                    ostringstream empty_tag;
                    empty_tag << ">  <empty>" << endl << endl << endl << ">  <non_empty>" << endl;
                    if (string::npos == s.find(empty_tag.str())) {
                        cerr << "Did not find expected empty tag in '" << s << "'." << endl;
                    }
                    CPPUNIT_ASSERT(string::npos != s.find(empty_tag.str()));
                    occurrences++;
                }
            }
            CPPUNIT_ASSERT_EQUAL(2, occurrences);
            inf.close();
        }
        
        void testTransformAndTagging() {
            // To be useful in align_monte, mol::SDWriter must be able to handle
            // changes in atom coordinates and addition of tags.
            
            string pathname("data/in/one_structure.sdf");
            ifstream inf(pathname.c_str());
            mol::SDReader reader(inf, pathname);
            mol::Mol m;
            CPPUNIT_ASSERT(reader.read(m));
            inf.close();
            
            // Take it on faith that the hydrogens are not all superposed
            // at (-100,-100,-100) in the input file.
            unsigned int hcount = 0;
            mol::AtomVector::const_iterator i;
            for (i = m.atoms().begin(); i != m.atoms().end(); ++i) {
                mol::Atom *a(*i);
                if (a->is_hydrogen()) {
                    a->x(-100.0);
                    a->y(-100.0);
                    a->z(-100.0);
                    hcount++;
                }
            }
            m.add_tag("test_sdwriter.hcount", hcount);
            m.add_tag("test_sdwriter.blank_terminated", "foo\n\n");
            m.add_tag("test_sdwriter.not_blank_termed", "foo");
            m.add_tag("test_sdwriter.multi_blank_termed", "foo    \n\n\n\n");
            
            ostringstream outs;
            mol::SDWriter writer(outs);
            writer.write(m);
            unsigned int written_hcount = 0;
            string s(outs.str());
            const string pattern(" -100.0000 -100.0000 -100.0000 H ");
            size_t i_start = 0;
            while (true) {
                i_start = s.find(pattern, i_start);
                if (i_start == string::npos) {
                    break;
                }
                i_start += pattern.size();
                written_hcount++;
            }
            CPPUNIT_ASSERT_EQUAL(hcount, written_hcount);
            
            {
                ostringstream tags;
                tags << ">  <test_sdwriter.hcount>" << endl << hcount << endl << endl;
                CPPUNIT_ASSERT(string::npos != s.find(tags.str()));
            }
            {
                ostringstream tags;
                tags << ">  <test_sdwriter.blank_terminated>" << endl << "foo" << endl << endl;
                CPPUNIT_ASSERT(string::npos != s.find(tags.str()));
            }
            {
                ostringstream tags;
                tags << ">  <test_sdwriter.not_blank_termed>" << endl << "foo" << endl << endl;
                CPPUNIT_ASSERT(string::npos != s.find(tags.str()));
            }
            {
                ostringstream tags;
                // Weak!  Doesn't test for too many blank lines.
                tags << ">  <test_sdwriter.multi_blank_termed>" << endl << "foo" << endl << endl;
                CPPUNIT_ASSERT(string::npos != s.find(tags.str()));
            }
            cout << "TODO:  Test for lines containing only whitespace." << endl;
        }
        
    protected:
        string fileContent(string& pathname) {
            ostringstream outs;
            const unsigned int length = 8192;
            char buffer[length];
            
            ifstream inf(pathname.c_str());
            while (inf) {
                inf.read(buffer, length-1);
                buffer[inf.gcount()] = '\0';
                outs << buffer;
            }
            
            return outs.str();
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
