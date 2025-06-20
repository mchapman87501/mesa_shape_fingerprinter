// Unit tests for mol_aligner.
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
#include <cstdlib>

#include "mesaac_shape/axis_aligner.h"
#include "mesaac_mol/element_info.h"

using namespace std;

namespace mesaac 
{
    using namespace shape;
    
    typedef PointList PL;  // Abbreviations
    typedef const PointList CPL;
    typedef mesaac::mol::AtomVector AV;
    
    class WBAxisAligner:  public AxisAligner {
    public:
        WBAxisAligner(PL& sphere, float atom_scale, bool atom_centers_only):
            AxisAligner(sphere, atom_scale, atom_centers_only)
        {}
        
        void wb_get_atom_points(const AV& atoms, PL& centers, 
            bool include_hydrogens)
        {
            get_atom_points(atoms, centers, include_hydrogens);
        }
        
        void wb_mean_center_points(PL& centers) {
            mean_center_points(centers);
        }
        
        void wb_get_mean_center(const PL& centers, Point& mean) {
            get_mean_center(centers, mean);
        }
        
        void wb_get_mean_centered_cloud(const PL& centers, PL& cloud) {
            get_mean_centered_cloud(centers, cloud);
        }
        
        void wb_find_axis_align_transform(const PL& cloud, Transform& t) {
            find_axis_align_transform(cloud, t);
        }
        
        void wb_untranslate_points(PL& all_centers, const Point& offset) {
            untranslate_points(all_centers, offset);
        }
        
        void wb_transform_points(PL& all_centers, Transform& t)  {
            transform_points(all_centers, t);
        }
        
        void wb_update_atom_coords(AV& atoms, const PL& all_centers) {
            update_atom_coords(atoms, all_centers);
        }
    };
    
    class TestCase : public CppUnit::TestFixture
    {
        CPPUNIT_TEST_SUITE(TestCase);

        CPPUNIT_TEST(test_get_atom_points_empty);
        CPPUNIT_TEST(test_get_atom_points);
        
        CPPUNIT_TEST(test_get_mean_center_empty);
        CPPUNIT_TEST(test_get_mean_center);

        CPPUNIT_TEST(test_mean_center_points_empty);
        CPPUNIT_TEST(test_mean_center_points);

        CPPUNIT_TEST(test_get_mean_centered_cloud_empty);
        CPPUNIT_TEST(test_get_mean_centered_cloud);

        CPPUNIT_TEST(test_find_axis_align_transform);
        CPPUNIT_TEST(test_untranslate_points);
        CPPUNIT_TEST(test_transform_points);
        CPPUNIT_TEST(test_update_atom_coords);
        
        CPPUNIT_TEST(test_align_to_axes);
        CPPUNIT_TEST(test_align_to_axes_mol);
        CPPUNIT_TEST(test_align_to_axes_mol_atoms_only);
        CPPUNIT_TEST(test_align_hydrogens);

        CPPUNIT_TEST(test_overlapping_spherules);

        // Benchmarks -- comment these out for production.
        CPPUNIT_TEST(benchmark_align_to_axes);
        
        CPPUNIT_TEST_SUITE_END();
        
    protected:
        bool almost_equal(float expected, float actual, 
                          float eps=1.0e-6, bool verbose=false)
        {
            bool result = (::fabs(expected - actual) <= eps);
            if (!result && verbose) {
                cout << "almost_equal(" << expected << ", " << actual 
                     << ", " << eps << ")" << endl
                     << "    Actual difference: " << ::fabs(expected - actual)
                     << endl
                     ;
            }
            return result;
        }
        
        void get_point_means(const PointList& points,
            float& x, float& y, float& z)
        {
            x = y = z = 0.0;
            if (points.size()) {
                PointList::const_iterator i;
                float xsum = 0, ysum = 0, zsum = 0;
                for (i = points.begin(); i != points.end(); ++i) {
                    const Point& p(*i);
                    xsum += p[0];
                    ysum += p[1];
                    zsum += p[2];
                }
                x = xsum / points.size();
                y = ysum / points.size();
                z = zsum / points.size();
            }
        }
        
        // Find the extent of set of points, along each axis.
        void get_point_extents(const PointList& points,
            float& dx, float& dy, float& dz)
        {
            dx = dy = dz = 0.0;
            if (points.size()) {
                PointList::const_iterator i;
                float xmin = 0, ymin = 0, zmin = 0,
                      xmax = 0, ymax = 0, zmax = 0;
                for (i = points.begin(); i != points.end(); ++i) {
                    const Point& p(*i);
                    if (i == points.begin()) {
                        xmin = xmax = p[0];
                        ymin = ymax = p[1];
                        zmin = zmax = p[2];
                    } else {
                        xmin = (xmin < p[0]) ? xmin : p[0];
                        xmax = (xmax > p[0]) ? xmax : p[0];
                        ymin = (ymin < p[1]) ? ymin : p[1];
                        ymax = (ymax > p[1]) ? ymax : p[1];
                        zmin = (zmin < p[2]) ? zmin : p[2];
                        zmax = (zmax > p[2]) ? zmax : p[2];
                    }
                }
                dx = xmax - xmin;
                dy = ymax - ymin;
                dz = zmax - zmin;
            }
        }
        
        void read_test_points(string pathname, PointList& points)
        {
            pathname = string("../../../../test_data/hammersley/") + pathname;
            points.clear();
            ifstream inf(pathname.c_str());
            if (!inf) {
                ostringstream msg;
                msg << "Could not open " << pathname << " for reading." << endl;
                CPPUNIT_FAIL(msg.str());
            }
            float x, y, z;
            while (inf >> x >> y >> z) {
                Point fv;
                fv.push_back(x);
                fv.push_back(y);
                fv.push_back(z);
                points.push_back(fv);
            }
            inf.close();
        }
        
        WBAxisAligner *new_aligner() {
            PointList sphere;
            float atom_scale = 1.0;
            
            // Assume we will be run in a location fixed relative to
            // the data files.
            read_test_points("hamm_spheroid_10k_11rad.txt", sphere);
            return new WBAxisAligner(sphere, atom_scale, false);
        }
        
        WBAxisAligner *new_aligner_ac_only() {
            PointList sphere;
            float atom_scale = 1.0;
            read_test_points("hamm_spheroid_10k_11rad.txt", sphere);
            return new WBAxisAligner(sphere, atom_scale, true);
        }
        
        Point make_point(float x, float y, float z) {
            Point result;
            result.push_back(x);
            result.push_back(y);
            result.push_back(z);
            return result;
        }
        
        Point make_center(float x, float y, float z, float r) {
            Point result(make_point(x, y, z));
            result.push_back(r);
            return result;
        }
        
        void add_atom(mol::Mol& m, string symbol, 
            float x, float y, float z)
        {
            mol::Atom a;
            
            a.atomic_num(getAtomicNum(symbol));
            a.x(x);
            a.y(y);
            a.z(z);
            m.add_atom(a);
        }
        
        void create_sample_mol(mol::Mol& mol, unsigned int& num_heavies) {
            // Coordinates taken from first cox2_3d conformer.
            add_atom(mol, "C", 27.7051, 22.0403, 17.0243);
            add_atom(mol, "N", 26.4399, 22.0976, 16.4318);
            add_atom(mol, "C", 25.5381, 21.4424, 17.2831);
            add_atom(mol, "C", 26.2525, 20.9753, 18.3748);
            add_atom(mol, "C", 27.5943, 21.3608, 18.2218);
            add_atom(mol, "C", 24.0821, 21.3670, 17.1082);
            add_atom(mol, "C", 26.1324, 22.6824, 15.1634);
            add_atom(mol, "C", 23.4105, 22.2668, 16.2675);
            add_atom(mol, "C", 22.0220, 22.2007, 16.1197);
            add_atom(mol, "C", 21.2976, 21.2409, 16.8307);
            add_atom(mol, "C", 21.9509, 20.3402, 17.6750);
            add_atom(mol, "C", 23.3399, 20.4115, 17.8175);
            add_atom(mol, "C", 26.3695, 24.0457, 14.9358);
            add_atom(mol, "C", 26.0627, 24.6119, 13.6959);
            add_atom(mol, "C", 25.5236, 23.8179, 12.6910);
            add_atom(mol, "C", 25.2821, 22.4660, 12.9010);
            add_atom(mol, "C", 25.5848, 21.8942, 14.1391);
            add_atom(mol, "F", 25.2311, 24.3643, 11.5034);
            add_atom(mol, "C", 28.9655, 22.5443, 16.4025);
            add_atom(mol, "S", 19.5328, 21.1457, 16.6315);
            add_atom(mol, "O", 19.0413, 22.4851, 16.3229);
            add_atom(mol, "O", 18.9873, 20.4577, 17.7975);
            add_atom(mol, "C", 19.3258, 20.1152, 15.2035);
            add_atom(mol, "H", 25.8309, 20.4354, 19.2156);
            add_atom(mol, "H", 28.4100, 21.1477, 18.9041);
            add_atom(mol, "H", 23.9630, 23.0298, 15.7210);
            add_atom(mol, "H", 21.5209, 22.9035, 15.4575);
            add_atom(mol, "H", 21.3956, 19.5863, 18.2287);
            add_atom(mol, "H", 23.8404, 19.7079, 18.4815);
            add_atom(mol, "H", 26.7480, 24.6961, 15.7203);
            add_atom(mol, "H", 26.2341, 25.6696, 13.5167);
            add_atom(mol, "H", 24.8658, 21.8592, 12.1019);
            add_atom(mol, "H", 25.4187, 20.8273, 14.2701);
            add_atom(mol, "H", 29.8338, 22.0387, 16.8401);
            add_atom(mol, "H", 29.0870, 23.6157, 16.5858);
            add_atom(mol, "H", 28.9947, 22.3510, 15.3261);
            add_atom(mol, "H", 18.2555, 20.0118, 15.0126);
            add_atom(mol, "H", 19.7633, 19.1356, 15.4046);
            add_atom(mol, "H", 19.8115, 20.5898, 14.3489);
            num_heavies = 23;
        }
        
        void create_sample_atoms(mol::AtomVector& atoms, 
            unsigned int& num_heavies) 
        {
            mol::Mol m;
            
            create_sample_mol(m, num_heavies);
            // Take care to deep-copy all of the atom pointers.
            atoms.clear();
            const mol::AtomVector& src(m.atoms());
            mol::AtomVector::const_iterator i;
            for (i = src.begin(); i != src.end(); ++i) {
                atoms.push_back(new mol::Atom(**i));
            }
        }
        
        bool coords_match(const mol::AtomVector& atoms, const PointList& points,
                          unsigned int count)
        {
            // cout << "coords_match? " << endl
            //      << "  # atoms:    " << atoms.size() << endl
            //      << "  # points:   " << points.size() << endl
            //      << "  # to check: " << count << endl;
            if ((atoms.size() >= count) && (points.size() >= count)) {
                for (int i = count - 1; i >= 0; --i) {
                    const mol::Atom *a(atoms[i]);
                    const Point& p(points[i]);
                    if (!(a && 
                          (a->x() == p[0]) && (a->y() == p[1]) &&
                          (a->z() == p[2]))) {
                        return false;
                    }
                }
                return true;
            }
            return false;
        }
        
        void get_pointlist_info(const PointList& points,
            float& xmid, float& ymid, float& zmid,
            float& width, float& height, float& depth)
        {
            xmid = ymid = zmid = width = height = depth = 0.0;
            if (points.size()) {
                float xmin, ymin, zmin, xmax, ymax, zmax;
                float xsum = 0, ysum = 0, zsum = 0;
                PointList::const_iterator i = points.begin();
                const Point p(*i);
                xmin = xmax = p[0];
                ymin = ymax = p[1];
                zmin = zmax = p[2];
                for (; i != points.end(); ++i) {
                    const Point p(*i);
                    float x(p[0]), y(p[1]), z(p[2]);
                    xsum += x;
                    ysum += y;
                    zsum += z;
                    xmin = (xmin < x) ? xmin : x;
                    xmax = (xmax > x) ? xmax : x;
                    ymin = (ymin < y) ? ymin : y;
                    ymax = (ymax > y) ? ymax : y;
                    zmin = (zmin < z) ? zmin : z;
                    zmax = (zmax > z) ? zmax : z;
                }
                
                xmid = xsum / points.size();
                ymid = ysum / points.size();
                zmid = zsum / points.size();
                width = xmax - xmin;
                height = ymax - ymin;
                depth = zmax - zmin;
            }
        }
        
        bool is_mean_centered(const PointList& points) {
            float xmid, ymid, zmid, w, h, d;
            get_pointlist_info(points, xmid, ymid, zmid, w, h, d);
            bool result = (
                almost_equal(0.0, xmid, 0.0001)
                && almost_equal(0.0, ymid, 0.0001)
                && almost_equal(0.0, zmid, 0.0001)
            );
            return result;
        }
        
        bool has_nonincreasing_extents(const PointList& points) {
            float xmid, ymid, zmid, w, h, d;
            get_pointlist_info(points, xmid, ymid, zmid, w, h, d);
            bool result = ((w >= h) && (h >= d) && (d > 0));
            return result;
        }
        
        float find_max_radius(const PointList& points) {
            float result = 0.0;
            PointList::const_iterator i;
            for (i = points.begin(); i != points.end(); ++i) {
                // Each of these points *should* have a radius.
                const Point& p(*i);
                result = (result > p.at(3)) ? result : p.at(3);
            }
            return result;
        }
        
        bool is_non_null_transform(Transform& a) {
            // WEAK!
            int low_row = a.getlowbound(1);
            int high_row = a.gethighbound(1);
            int low_col = a.getlowbound(2);
            int high_col = a.gethighbound(2);
            bool result = (((high_row - low_row) == 2) &&
                           ((high_col - low_col) == 2));
            if (result) {
                result = false;
                for (int r = low_row; !result && (r <= high_row); r++) {
                    ap::raw_vector<double> row_v = a.getrow(
                        r, low_col, high_col);
                    double *curr_col = row_v.GetData();
                    for (int icol = low_col; !result && (icol <= high_col);
                         ++icol) {
                        result = (*curr_col != 0.0);
                        curr_col++;
                    }
                }
            }
            return result;
        }

    public:
        void test_get_atom_points_empty() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            PointList points;
            
            aligner->wb_get_atom_points(atoms, points, false);
            CPPUNIT_ASSERT_EQUAL((size_t)0, atoms.size());
            CPPUNIT_ASSERT_EQUAL((size_t)0, points.size());
            aligner->wb_get_atom_points(atoms, points, true);
            CPPUNIT_ASSERT_EQUAL((size_t)0, atoms.size());
            CPPUNIT_ASSERT_EQUAL((size_t)0, points.size());
        }
        
        void test_get_atom_points() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            PointList points;
            unsigned int num_heavies;
            
            create_sample_atoms(atoms, num_heavies);
            aligner->wb_get_atom_points(atoms, points, false);
            CPPUNIT_ASSERT(atoms.size() > num_heavies);
            CPPUNIT_ASSERT(num_heavies > 0);
            CPPUNIT_ASSERT(coords_match(atoms, points, num_heavies));
            
            aligner->wb_get_atom_points(atoms, points, true);
            CPPUNIT_ASSERT(coords_match(atoms, points, atoms.size()));
        }
        
        
        void test_get_mean_center_empty() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            PointList points;
            Point center;
            
            aligner->wb_get_mean_center(points, center);
            CPPUNIT_ASSERT_EQUAL((size_t)3, center.size());
            CPPUNIT_ASSERT_EQUAL(0.0f, center[0]);
            CPPUNIT_ASSERT_EQUAL(0.0f, center[1]);
            CPPUNIT_ASSERT_EQUAL(0.0f, center[2]);
        }
        
        void test_get_mean_center() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            PointList points;
            unsigned int num_heavies;
            Point center;
            
            create_sample_atoms(atoms, num_heavies);
            aligner->wb_get_atom_points(atoms, points, false);
            aligner->wb_get_mean_center(points, center);
            CPPUNIT_ASSERT_EQUAL((size_t)3, center.size());
            // FRAGILE
            CPPUNIT_ASSERT(almost_equal(24.1596, center[0], 0.0001));
            CPPUNIT_ASSERT(almost_equal(22.0163, center[1], 0.0001));
            CPPUNIT_ASSERT(almost_equal(15.9366, center[2], 0.0001));
        }
        

        void test_mean_center_points_empty() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            PointList points;

            CPPUNIT_ASSERT_EQUAL((size_t)0, points.size());
            aligner->wb_mean_center_points(points);
            // If we make it this far, we pass.
            CPPUNIT_ASSERT_EQUAL((size_t)0, points.size());
        }
        
        void test_mean_center_points() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            PointList points;
            unsigned int num_heavies;
            Point center;

            // TODO:  create a set of atoms w. known positions and
            // easily verified extents.
            create_sample_atoms(atoms, num_heavies);
            aligner->wb_get_atom_points(atoms, points, false);
            CPPUNIT_ASSERT_EQUAL((size_t)num_heavies, points.size());
            aligner->wb_mean_center_points(points);
            CPPUNIT_ASSERT_EQUAL((size_t)num_heavies, points.size());
            CPPUNIT_ASSERT(is_mean_centered(points));
            
            float xmid, ymid, zmid, width, height, depth;
            get_pointlist_info(points, xmid, ymid, zmid, width, height, depth);
            CPPUNIT_ASSERT(almost_equal(9.9782, width, 0.00001));
            CPPUNIT_ASSERT(almost_equal(4.4967, height, 0.00001));
            CPPUNIT_ASSERT(almost_equal(6.8714, depth, 0.00001));
        }
        
        void test_get_mean_centered_cloud_empty() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            PointList points, cloud;
            CPPUNIT_ASSERT_EQUAL((size_t)0, points.size());
            aligner->wb_get_mean_centered_cloud(points, cloud);
            // If we get here without crashing, we win.
            CPPUNIT_ASSERT_EQUAL((size_t)0, cloud.size());
        }
        
        void test_get_mean_centered_cloud() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            PointList points, cloud;
            unsigned int num_heavies;

            create_sample_atoms(atoms, num_heavies);
            aligner->wb_get_atom_points(atoms, points, false);
            CPPUNIT_ASSERT(points.size() > 0);
            CPPUNIT_ASSERT_EQUAL((size_t)num_heavies, points.size());
            aligner->wb_mean_center_points(points);
            CPPUNIT_ASSERT_EQUAL((size_t)num_heavies, points.size());
            aligner->wb_get_mean_centered_cloud(points, cloud);
            CPPUNIT_ASSERT(cloud.size() > 0);

            float xmid, ymid, zmid, pwidth, pheight, pdepth;
            get_pointlist_info(points, xmid, ymid, zmid, 
                               pwidth, pheight, pdepth);
            CPPUNIT_ASSERT(is_mean_centered(points));
            CPPUNIT_ASSERT(almost_equal(9.9782, pwidth, 0.00001));
            CPPUNIT_ASSERT(almost_equal(4.4967, pheight, 0.00001));
            CPPUNIT_ASSERT(almost_equal(6.8714, pdepth, 0.00001));

            float cwidth, cheight, cdepth;
            get_pointlist_info(cloud, xmid, ymid, zmid, 
                               cwidth, cheight, cdepth);
            float dmax = 2.0 * find_max_radius(points);
            // Bench-check:  max radius should be 1.8, for sulfur.
            CPPUNIT_ASSERT_EQUAL(3.6f, dmax);
            
            float dwidth = cwidth - pwidth,
                  dheight = cheight - pheight,
                  ddepth = cdepth - pdepth;
                  
            CPPUNIT_ASSERT(dwidth > 0);
            CPPUNIT_ASSERT(dwidth <= dmax);
            CPPUNIT_ASSERT(dheight > 0);
            CPPUNIT_ASSERT(dheight <= dmax);
            CPPUNIT_ASSERT(ddepth > 0);
            CPPUNIT_ASSERT(ddepth <= dmax);
        }
        
        void test_find_axis_align_transform() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            PointList points, cloud;
            unsigned int num_heavies;

            create_sample_atoms(atoms, num_heavies);
            aligner->wb_get_atom_points(atoms, points, false);
            aligner->wb_mean_center_points(points);
            aligner->wb_get_mean_centered_cloud(points, cloud);

            // Not sure how to test this.  Just confirm it's a 3x3 matrix
            // with non-empty cells?
            Transform transform;
            CPPUNIT_ASSERT(!is_non_null_transform(transform));
            aligner->wb_find_axis_align_transform(cloud, transform);
            CPPUNIT_ASSERT(is_non_null_transform(transform));
        }
        
        void test_untranslate_points() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            Point offset = make_point(1.0, 2.0, 3.0);
            
            PointList points;
            
            // Ensure proper handling of empty lists:
            aligner->wb_untranslate_points(points, offset);
            CPPUNIT_ASSERT_EQUAL((size_t)0, points.size());
            
            unsigned int i;
            const unsigned int i_max = 10;
            for (i = 0; i != i_max; i++) {
                points.push_back(make_point(i + 1.0, i + 2.0, i + 3.0));
            }
            
            aligner->wb_untranslate_points(points, offset);
            for (i = 0; i != i_max; i++) {
                float f(i);
                Point& p(points[i]);
                CPPUNIT_ASSERT_EQUAL(f, p.at(0));
                CPPUNIT_ASSERT_EQUAL(f, p.at(1));
                CPPUNIT_ASSERT_EQUAL(f, p.at(2));
            }
        }
        
        void test_transform_points() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            PointList points, cloud;
            unsigned int num_heavies;
            Transform transform;

            // Ensure no crash on empty cloud:
            CPPUNIT_ASSERT_THROW(
                aligner->wb_find_axis_align_transform(cloud, transform),
                invalid_argument);
            
            create_sample_atoms(atoms, num_heavies);
            aligner->wb_get_atom_points(atoms, points, false);
            aligner->wb_mean_center_points(points);
            aligner->wb_get_mean_centered_cloud(points, cloud);

            CPPUNIT_ASSERT(!is_non_null_transform(transform));
            aligner->wb_find_axis_align_transform(cloud, transform);
            CPPUNIT_ASSERT(is_non_null_transform(transform));

            // Verify no crash on an empty point list.
            points.clear();
            aligner->wb_transform_points(points, transform);
            CPPUNIT_ASSERT_EQUAL((size_t) 0, points.size());
            
            aligner->wb_get_atom_points(atoms, points, false);
            CPPUNIT_ASSERT(!has_nonincreasing_extents(points));
            
            aligner->wb_transform_points(points, transform);
            CPPUNIT_ASSERT_EQUAL((size_t)num_heavies, points.size());

            // Transform should merely rotate the points -- no mean-centering.
            CPPUNIT_ASSERT(has_nonincreasing_extents(points));
        }
        
        void test_update_atom_coords() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            PointList points, cloud;
            unsigned int num_heavies;

            // Ensure correct copying of transformed point coords to
            // corresponding atom coords.
            create_sample_atoms(atoms, num_heavies);
            
            // Point and atom lists of different size?  This should fail.
            aligner->wb_get_atom_points(atoms, points, false);
            CPPUNIT_ASSERT_THROW(
                aligner->wb_update_atom_coords(atoms, points),
                std::length_error);
            
            // Moronic, but maybe adequate, test: superpose all atoms.
            aligner->wb_get_atom_points(atoms, points, true);
            Point offset(make_point(10.0, -50.0, 0.0));
            PointList::iterator i;
            for (i = points.begin(); i != points.end(); ++i) {
                Point& p(*i);
                p.at(0) = offset[0];
                p.at(1) = offset[1];
                p.at(2) = offset[2];
            }
            aligner->wb_update_atom_coords(atoms, points);

            mol::AtomVector::const_iterator j;
            for (j = atoms.begin(); j != atoms.end(); ++j) {
                const mol::Atom& a(**j);
                CPPUNIT_ASSERT_EQUAL(offset[0], a.x());
                CPPUNIT_ASSERT_EQUAL(offset[1], a.y());
                CPPUNIT_ASSERT_EQUAL(offset[2], a.z());
            }
        }
        
        void test_align_to_axes() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::AtomVector atoms;
            
            // No crash on empty:
            aligner->align_to_axes(atoms);
            CPPUNIT_ASSERT_EQUAL((size_t)0, atoms.size());
            
            PointList points, cloud;
            unsigned int num_heavies;

            create_sample_atoms(atoms, num_heavies);
            aligner->wb_get_atom_points(atoms, points, false);
            CPPUNIT_ASSERT(!is_mean_centered(points));
            CPPUNIT_ASSERT(!has_nonincreasing_extents(points));

            // TODO:  Check that the hydrogens are also transformed.
            aligner->align_to_axes(atoms);
            aligner->wb_get_atom_points(atoms, points, false);
            CPPUNIT_ASSERT(is_mean_centered(points));
            CPPUNIT_ASSERT(has_nonincreasing_extents(points));
        }
        
        void test_align_to_axes_mol() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::Mol mol;
            
            // No crash on empty:
            aligner->align_to_axes(mol);
            CPPUNIT_ASSERT_EQUAL((size_t)0, mol.atoms().size());
            
            PointList points, cloud;
            unsigned int num_heavies;

            create_sample_mol(mol, num_heavies);
            aligner->wb_get_atom_points(mol.atoms(), points, false);
            CPPUNIT_ASSERT(!is_mean_centered(points));
            CPPUNIT_ASSERT(!has_nonincreasing_extents(points));

            // TODO:  Check that the hydrogens are also transformed.
            aligner->align_to_axes(mol);
            aligner->wb_get_atom_points(mol.atoms(), points, false);
            CPPUNIT_ASSERT(is_mean_centered(points));
            CPPUNIT_ASSERT(has_nonincreasing_extents(points));
        }
        
        void benchmark_align_to_axes() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            auto_ptr<WBAxisAligner> ac_aligner(new_aligner_ac_only());
            cout << "Cloud:\t" << _benchmark_align_to_axes(aligner) 
                << " alignments/clock" << endl;
            cout << "Centers:\t" << _benchmark_align_to_axes(ac_aligner) 
                << " alignments/clock" << endl;
        }
        
        float _benchmark_align_to_axes(auto_ptr<WBAxisAligner>& aligner) {
            clock_t t0 = clock();
            const int NumTrials = 1000;
            for (int i = 0; i < NumTrials; i++) {
                mol::Mol mol;
                PointList points, cloud;
                unsigned int num_heavies;

                create_sample_mol(mol, num_heavies);
                aligner->wb_get_atom_points(mol.atoms(), points, false);

                // TODO:  Check that the hydrogens are also transformed.
                aligner->align_to_axes(mol);
                aligner->wb_get_atom_points(mol.atoms(), points, false);
            }
            clock_t tf = clock();
            float dt = (tf - t0);
            return NumTrials / dt;
        }
        void test_align_to_axes_mol_atoms_only() {
            auto_ptr<WBAxisAligner> aligner(new_aligner_ac_only());
            mol::Mol mol;
            
            // No crash on empty:
            aligner->align_to_axes(mol);
            CPPUNIT_ASSERT_EQUAL((size_t)0, mol.atoms().size());
            
            PointList points, cloud;
            unsigned int num_heavies;

            create_sample_mol(mol, num_heavies);
            aligner->wb_get_atom_points(mol.atoms(), points, false);
            CPPUNIT_ASSERT(!is_mean_centered(points));
            CPPUNIT_ASSERT(!has_nonincreasing_extents(points));

            // TODO:  Check that the hydrogens are also transformed.
            // TODO:  Check that align_to_axes with atom-centers-only produces
            // different results than without atom-centers-only.
            aligner->align_to_axes(mol);
            aligner->wb_get_atom_points(mol.atoms(), points, false);
            CPPUNIT_ASSERT(is_mean_centered(points));
            CPPUNIT_ASSERT(has_nonincreasing_extents(points));
        }

        // This is pretty naive: lay out some atoms in a line.
        // Confirm that the line, including its hydrogens, gets mapped onto
        // the X axis.
        void test_align_hydrogens() {
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            mol::Mol mol;
            PointList points;
            float x, y, z, w, h, d;

            add_atom(mol, "C", 0.0, 0.0, 0.0); // Stay inside the point cloud
            add_atom(mol, "H", 0.0, 0.0, 1.0);
            add_atom(mol, "C", 0.0, 0.0, 2.0);
            add_atom(mol, "H", 0.0, 0.0, 3.0);
            add_atom(mol, "C", 0.0, 0.0, 4.0);
            add_atom(mol, "H", 0.0, 0.0, 5.0);
            add_atom(mol, "C", 0.0, 0.0, 6.0);
            add_atom(mol, "H", 0.0, 0.0, 7.0);
            
            aligner->wb_get_atom_points(mol.atoms(), points, true);
            get_pointlist_info(points, x, y, z, w, h, d);
            CPPUNIT_ASSERT_EQUAL(0.0f, w);
            CPPUNIT_ASSERT_EQUAL(0.0f, h);
            CPPUNIT_ASSERT_EQUAL(7.0f, d);
            
            aligner->align_to_axes(mol);
            aligner->wb_get_atom_points(mol.atoms(), points, true);
            
            get_pointlist_info(points, x, y, z, w, h, d);
            
            // The slope from point to point should be consistent.
            const float dx_dy = w / h;
            const float dx_dz = w / d;
            const float dy_dz = h / d;
            const mol::AtomVector& atoms(mol.atoms());
            mol::AtomVector::const_iterator i;
            mol::AtomVector::const_iterator iprev = atoms.end();
            for (i = atoms.begin(); i != atoms.end(); ++i) {
                if (iprev != atoms.end()) {
                    const mol::Atom& prev(**iprev);
                    const mol::Atom& curr(**i);
                    // Not the best test -- dunno the expected direction:
                    float dx = ::fabs(curr.x() - prev.x());
                    float dy = ::fabs(curr.y() - prev.y());
                    float dz = ::fabs(curr.z() - prev.z());
                    CPPUNIT_ASSERT(almost_equal(dx_dy, dx/dy, 0.00014, true));
                    CPPUNIT_ASSERT(almost_equal(dx_dz, dx/dz, 0.00014, true));
                    CPPUNIT_ASSERT(almost_equal(dy_dz, dy/dz, 0.00014, true));
                }
                iprev = i;
            }

            // The axis alignment is not perfect.
            CPPUNIT_ASSERT(almost_equal(7.0, w, 0.001));
            CPPUNIT_ASSERT(almost_equal(0.0, h, 0.0125));
            CPPUNIT_ASSERT(almost_equal(0.0, d, 0.05));
        }


        void test_overlapping_spherules() {
            // This is a simple test which demonstrates that,
            // if two spherules overlap, their corresponding cloud points
            // will not be double-counted.
            
            const Point atom(make_center(0, 0, 0, 1.7));
            
            PointList mol1, mol2;
            mol1.push_back(atom);
            // Duplicates, superposed!
            mol2.push_back(atom);
            mol2.push_back(atom);
            
            auto_ptr<WBAxisAligner> aligner(new_aligner());
            PointList contained1, contained2;
            aligner->wb_get_mean_centered_cloud(mol1, contained1);
            aligner->wb_get_mean_centered_cloud(mol2, contained2);
            
            CPPUNIT_ASSERT(contained1 == contained2);
            CPPUNIT_ASSERT(contained1.size() > 0);
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
