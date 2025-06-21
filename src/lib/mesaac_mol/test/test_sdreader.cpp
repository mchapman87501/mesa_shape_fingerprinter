// Unit test for mol::SDReader.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <string>
#include <iostream>
#include <fstream>
#include <stdexcept>

// Confirm that the top-level include really pulls in all mesa_mol
// headers:
#include "mesaac_mol.h"


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
    
    class WhiteBoxMol: public mol::Mol {
    public:
        unsigned int num_bonds() {
            return m_bonds.size();
        }
        
        string tagstr() {
            ostringstream resultf;
            mol::SDTagMap::iterator i;
            // Maps are supposed to keep their keys in sorted order.
            for (i = m_tags.begin(); i != m_tags.end(); i++) {
                resultf << "'" << i->first << "' = '" << i->second << "'" 
                        << endl;
            }
            return resultf.str();
        }
    };
    
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testOneStructure);
        CPPUNIT_TEST(testMultipleStructures);
        CPPUNIT_TEST(testPropertiesBlock);
        CPPUNIT_TEST(testTags);
        CPPUNIT_TEST(testGarbageInput);
        
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
            
        }

        void tearDown() {
        }

        void testOneStructure() {
            // OBS:  Assumes test is run in the directory containing
            // this source file.
            // If we ever port to CMake for out-of-line builds, we'll
            // need to specify the desired working directory for CPPUNIT
            // executables:
            // add_test(MyTest ${CMAKE_COMMAND} -E chdir ${MyDir} testexex)
            // (via http://www.mail-archive.com/cmake@cmake.org/msg24198.html)
            string pathname("data/in/one_structure.sdf");
            ifstream inf(pathname.c_str());
            mol::SDReader reader(inf, pathname);
            unsigned int num_found = 0;
            WhiteBoxMol m;
            // O for C++0x and its static initializers.
            const char *expected_symbols[] = {
                "C", "N", 
                "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", "C", 
                "C", "C",
                "F", "C", "S", "O", "O", "C", 
                "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", "H", 
                "H", "H", "H", 
            };
            while (reader.read(m)) {
                // First (and last) molecule should have the expected
                // properties.
                CPPUNIT_ASSERT_EQUAL(39u, m.num_atoms());
                CPPUNIT_ASSERT_EQUAL(23u, m.num_heavy_atoms());
                CPPUNIT_ASSERT_EQUAL(41u, m.num_bonds());
                // No bounds check -- if it crashes, the test fails :)
                mol::AtomVector::const_iterator a;
                unsigned int i;
                for (a = m.atoms().begin(), i = 0; a != m.atoms().end(); a++, i++) {
                    CPPUNIT_ASSERT_EQUAL(
                        (*a)->symbol(), string(expected_symbols[i]));
                }
                
                // Verify expected info for first atom:
                // 27.7051   22.0403   17.0243 C   0  0  0  0  0  0

                a = m.atoms().begin();
                mol::Atom *atom(*a);
                CPPUNIT_ASSERT_EQUAL(27.7051f, atom->x());
                CPPUNIT_ASSERT_EQUAL(22.0403f, atom->y());
                CPPUNIT_ASSERT_EQUAL(17.0243f, atom->z());
                CPPUNIT_ASSERT_EQUAL(string("C"), atom->symbol());
                CPPUNIT_ASSERT_EQUAL(string(" 0  0  0  0  0  0"), atom->optional_cols());
                
                num_found++;
            }
            inf.close();
            CPPUNIT_ASSERT_EQUAL(1u, num_found);
        }
        
        void testMultipleStructures() {
            string pathname("data/in/cox2_3d.sd");
            // Spot-check some atom and bond counts.
            unsigned int mol_check_indices[] = {
                  0,  10, 456, 466
            };
            const unsigned int num_to_check =
                sizeof(mol_check_indices)/sizeof(mol_check_indices[0]);
            
            unsigned int expected_num_atoms[] = {
                 39,  45,  55, 36
            };
            unsigned int expected_num_bonds[] = {
                 41,  47,  58, 38,
            };
            
            ifstream inf(pathname.c_str());
            mol::SDReader reader(inf, pathname);
            
            unsigned int num_found = 0;
            unsigned int i_check = 0;
            unsigned int *check_index = mol_check_indices;
            WhiteBoxMol m;
            
            while (reader.read(m)) {
                while ((i_check < num_to_check) && 
                       (num_found > *check_index)) {
                    i_check++;
                    check_index++;
                }
                if ((i_check < num_to_check) &&
                    (num_found == *check_index)) {
                    CPPUNIT_ASSERT_EQUAL(expected_num_atoms[i_check],
                        m.num_atoms());
                    CPPUNIT_ASSERT_EQUAL(expected_num_bonds[i_check],
                        m.num_bonds());
                    i_check++;
                    check_index++;
                }
                num_found++;
            }
            CPPUNIT_ASSERT_EQUAL(467u, num_found);
        }
        
        void testPropertiesBlock() {
            // Show we can read properties blocks.
            string pathname("data/in/property_blocks.sdf");
            ifstream inf(pathname.c_str());
            mol::SDReader reader(inf, pathname);
            
            WhiteBoxMol m;

            // Check the first and last molecules.
            CPPUNIT_ASSERT(reader.read(m));
            const string exp_first_block("M  CHG  2   2   1  19  -1\nM  END\n");
            CPPUNIT_ASSERT_EQUAL(exp_first_block, m.properties_block());

            string last_properties_block(m.properties_block());
            while (reader.read(m)) {
                last_properties_block = m.properties_block();
            }
            
            const string exp_last_block("M  CHG  2  31   1  33  -1\nM  END\n");
            CPPUNIT_ASSERT_EQUAL(exp_last_block, last_properties_block);
        }
        
        void testTags() {
            string pathname("data/in/property_blocks.sdf");
            ifstream inf(pathname.c_str());
            mol::SDReader reader(inf, pathname);
            
            WhiteBoxMol m;

            // Check the first and last molecules.
            CPPUNIT_ASSERT(reader.read(m));
            const string exp_first("");
            if (exp_first != m.tagstr()) {
                cerr << "tag strings don't match:" << endl
                     << "Diff    : "
                     << strdiff_summary(exp_first, m.tagstr())
                     << endl;
            }
            CPPUNIT_ASSERT_EQUAL(exp_first, m.tagstr());

            string prev(m.tagstr());
            while (reader.read(m)) {
                prev = m.tagstr();
            }
            
            const string exp_last("'>   (MD-0894)	<BOILING.POINT>	FROM ARCHIVES' = 'Yet another example, with extensive whitespace after the '> '.\n'\n'> (MD-0894)	<BOILING.POINT>	FROM ARCHIVES' = 'Yet another example.\n'\n'> <Attribute>' = '1\n'\n'> <ID>' = '644742\n'\n'> DT12	55' = 'Another sample\nwith multiline values.\n'\n'>55 (MD-08974)	<BOILING.POINT>	DT12' = 'This is a sample tag from the ctfile spec.\n'\n");
            if (exp_last != prev) {
                cerr << "tag strings don't match:" << endl
                     << "Diff    : "
                     << strdiff_summary(exp_last, prev)
                     << endl;
            }
            CPPUNIT_ASSERT_EQUAL(exp_last, prev);
        }
        
        void testGarbageInput() {
            // Try reading from a corrupt SD file, one in which newlines
            // have been smooshed into spaces.
            string pathname("data/in/corrupt.sdf");
            ifstream inf(pathname.c_str());
            mol::SDReader reader(inf, pathname);
            unsigned int num_found = 0;
            WhiteBoxMol m;
            while (reader.read(m)) {
                CPPUNIT_ASSERT(m.num_atoms() > 0);
                num_found++;
            }
            inf.close();
            CPPUNIT_ASSERT_EQUAL(0u, num_found);
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
