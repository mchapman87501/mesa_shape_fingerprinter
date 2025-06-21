// Unit test for mol::Mol
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/ui/text/TestRunner.h>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include "mesaac_mol/mol.h"

using namespace std;

namespace mesaac 
{
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);
        CPPUNIT_TEST(testBasic);
        CPPUNIT_TEST(testAtomsAndBonds);
        CPPUNIT_TEST(testProperties);
        CPPUNIT_TEST(testTags);
        
        CPPUNIT_TEST_SUITE_END();

    public:
        void setUp() {
        }

        void tearDown() {
        }

        void testBasic() {
            mol::Mol m1;
            CPPUNIT_ASSERT_EQUAL(string("<br/><br/>"), header_block(m1));
            m1.name("A molecule");
            m1.metadata("ill-formed");
            m1.comments("t&t");
            CPPUNIT_ASSERT_EQUAL(string("A molecule<br/>ill-formed<br/>t&t"),
                header_block(m1));
            m1.clear();
            CPPUNIT_ASSERT_EQUAL(string("<br/><br/>"), header_block(m1));
        }
        
        void testAtomsAndBonds() {
            mol::Mol mol;
            const unsigned int C_NumAtoms = 10;
            unsigned int i;
            for (i = 0; i < C_NumAtoms; i++) {
                mol::Atom a;
                a.atomic_num(i);
                a.x(i);
                mol.add_atom(a);
            }
            CPPUNIT_ASSERT_EQUAL(C_NumAtoms, mol.num_atoms());
            CPPUNIT_ASSERT_EQUAL(C_NumAtoms - 1, mol.num_heavy_atoms());
            
            unsigned int visited = 0;
            mol::AtomVector::const_iterator j;
            for (j = mol.atoms().begin(); j != mol.atoms().end(); j++) {
                mol::Atom *a = (*j);
                CPPUNIT_ASSERT(a);
                CPPUNIT_ASSERT_EQUAL(visited, a->atomic_num());
                CPPUNIT_ASSERT_EQUAL((float)visited, a->x());
                CPPUNIT_ASSERT_EQUAL(0.0f, a->y());
                CPPUNIT_ASSERT_EQUAL(0.0f, a->z());
                visited++;
            }
            CPPUNIT_ASSERT_EQUAL(C_NumAtoms, visited);
            
            mol::Bond b_orig;
            b_orig.a0(1);  // Bond numbers are one-based.
            b_orig.a1(2);
            b_orig.type(mol::Bond::BTE_AROMATIC);
            b_orig.stereo(mol::Bond::BSE_CIS_TRANS_DOUBLE);
            b_orig.optional_cols("xxxrrrccc");
            mol.add_bond(b_orig);
            mol::BondVector::const_iterator k;
            visited = 0;
            for (k = mol.bonds().begin(); k != mol.bonds().end(); k++) {
                mol::Bond *b = (*k);
                CPPUNIT_ASSERT(b);
                CPPUNIT_ASSERT(b != &b_orig);
                CPPUNIT_ASSERT_EQUAL(1U, b->a0());
                CPPUNIT_ASSERT_EQUAL(2U, b->a1());
                CPPUNIT_ASSERT_EQUAL((int)mol::Bond::BTE_AROMATIC, (int)b->type());
                CPPUNIT_ASSERT_EQUAL((int)mol::Bond::BSE_CIS_TRANS_DOUBLE, (int)b->stereo());
                CPPUNIT_ASSERT_EQUAL(string("xxxrrrccc"), b->optional_cols());
                visited++;
            }
            CPPUNIT_ASSERT_EQUAL(1u, visited);
            
            mol.clear();
            CPPUNIT_ASSERT_EQUAL(0U, mol.num_atoms());
            CPPUNIT_ASSERT(mol.bonds().begin() == mol.bonds().end());
        }
        
        void testProperties() {
            mol::Mol mol;
            
            CPPUNIT_ASSERT_EQUAL(string(""), mol.properties_block());
            string pb("M  1\nM  2\nM  END\n");
            mol.properties_block(pb);
            CPPUNIT_ASSERT_EQUAL(pb, mol.properties_block());
            mol.clear();
            CPPUNIT_ASSERT_EQUAL(string(""), mol.properties_block());
        }
        
        void testTags() {
            mol::Mol mol;
            CPPUNIT_ASSERT_EQUAL((size_t)0, mol.tags().size());
            mol.add_tag("t1", 1.0);
            mol.add_tag("t2", 2.0);
            CPPUNIT_ASSERT_EQUAL((size_t)2, mol.tags().size());
            
            const string FirstValue("3 and something");
            mol.add_tag("t3", FirstValue);
            CPPUNIT_ASSERT_EQUAL((size_t)3, mol.tags().size());

            // XXX FIX THIS:  you can't retrieve a tag using the
            // same syntax as you used to set it...
            mol::SDTagMap::const_iterator it = mol.tags().find(">  <t3>");
            CPPUNIT_ASSERT(it != mol.tags().end());
            CPPUNIT_ASSERT_EQUAL(it->second, FirstValue);
            
            // Duplicate
            mol.add_tag("t3", 42.0);
            it = mol.tags().find(">  <t3>");
            CPPUNIT_ASSERT(it != mol.tags().end());
            istringstream ins(it->second);
            float v;
            ins >> v;
            CPPUNIT_ASSERT_EQUAL(42.0f, v);
        }
        
    protected:
        string header_block(mol::Mol& m) {
            ostringstream outs;
            outs << m.name() << "<br/>" << m.metadata() << "<br/>" << m.comments();
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
