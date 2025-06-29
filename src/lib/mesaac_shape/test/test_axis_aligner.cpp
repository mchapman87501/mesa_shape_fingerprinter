// Unit tests for AxisAligner.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

// TODO share this code with lib/mesaac_shape_eigen/test.

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <fstream>

#include "mesaac_mol/element_info.hpp"
#include "mesaac_shape/axis_aligner.hpp"

using namespace std;

namespace mesaac::shape {

namespace {
class WBAxisAligner : public AxisAligner {
public:
  WBAxisAligner(PointList &sphere, float atom_scale, bool atom_centers_only)
      : AxisAligner(sphere, atom_scale, atom_centers_only) {}

  void wb_get_atom_points(const mol::AtomVector &atoms, PointList &centers,
                          bool include_hydrogens) {
    get_atom_points(atoms, centers, include_hydrogens);
  }

  void wb_mean_center_points(PointList &centers) {
    mean_center_points(centers);
  }

  void wb_get_mean_center(const PointList &centers, Point &mean) {
    get_mean_center(centers, mean);
  }

  void wb_get_mean_centered_cloud(const PointList &centers, PointList &cloud) {
    get_mean_centered_cloud(centers, cloud);
  }

  void wb_find_axis_align_transform(const PointList &cloud, Transform &t) {
    find_axis_align_transform(cloud, t);
  }

  void wb_untranslate_points(PointList &all_centers, const Point &offset) {
    untranslate_points(all_centers, offset);
  }

  void wb_transform_points(PointList &all_centers, Transform &t) {
    transform_points(all_centers, t);
  }

  void wb_update_atom_coords(mol::AtomVector &atoms,
                             const PointList &all_centers) {
    update_atom_coords(atoms, all_centers);
  }
};

class TestFixture {
public:
  void get_point_means(const PointList &points, float &x, float &y, float &z) {
    x = y = z = 0.0;
    if (points.size()) {
      float xsum = 0, ysum = 0, zsum = 0;
      for (const auto p : points) {
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
  void get_point_extents(const PointList &points, float &dx, float &dy,
                         float &dz) {
    dx = dy = dz = 0.0;
    if (points.size()) {
      bool first = true;
      float xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
      for (const Point &p : points) {
        if (first) {
          xmin = xmax = p[0];
          ymin = ymax = p[1];
          zmin = zmax = p[2];
          first = false;
        } else {
          xmin = min(xmin, p[0]);
          xmax = max(xmax, p[0]);
          ymin = min(ymin, p[1]);
          ymax = max(ymax, p[1]);
          zmin = min(zmin, p[2]);
          zmax = max(zmax, p[2]);
        }
      }
      dx = xmax - xmin;
      dy = ymax - ymin;
      dz = zmax - zmin;
    }
  }

  void read_test_points(const string pathname, PointList &points) {
    // TODO use std::filesystem::path, available since C++17.
    // Use the test data directory specified by TEST_DATA_DIR preprocessor
    // symbol.
    const string test_data_dir(TEST_DATA_DIR);
    const string data_dir(test_data_dir + "/hammersley/");
    const string full_path(data_dir + pathname);
    points.clear();
    ifstream inf(full_path.c_str());
    if (!inf) {
      ostringstream msg;
      msg << "Could not open " << pathname << " for reading." << endl;
      throw std::runtime_error(msg.str());
    }
    float x, y, z;
    while (inf >> x >> y >> z) {
      points.push_back({x, y, z});
    }
    inf.close();
  }

  std::unique_ptr<WBAxisAligner> new_aligner() {
    PointList sphere;
    float atom_scale = 1.0;

    // Assume we will be run in a location fixed relative to
    // the data files.
    read_test_points("hamm_spheroid_10k_11rad.txt", sphere);
    return std::make_unique<WBAxisAligner>(sphere, atom_scale, false);
  }

  std::unique_ptr<WBAxisAligner> new_aligner_ac_only() {
    PointList sphere;
    float atom_scale = 1.0;
    read_test_points("hamm_spheroid_10k_11rad.txt", sphere);
    return std::make_unique<WBAxisAligner>(sphere, atom_scale, true);
  }

  void add_atom(mol::Mol &m, string symbol, float x, float y, float z) const {
    const unsigned char atomic_num(mol::get_atomic_num(symbol));
    mol::Atom a(atomic_num, {x, y, z});
    m.add_atom(a);
  }

  void create_sample_mol(mol::Mol &mol, unsigned int &num_heavies) const {
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

  void create_sample_atoms(mol::AtomVector &atoms, unsigned int &num_heavies) {
    mol::Mol m;

    create_sample_mol(m, num_heavies);
    // Take care to deep-copy all of the atom pointers.
    atoms.clear();
    const mol::AtomVector &src(m.atoms());
    for (const auto src_atom : src) {
      atoms.push_back(src_atom);
    }
  }

  bool coords_match(const mol::AtomVector &atoms, const PointList &points,
                    unsigned int count) {
    // cout << "coords_match? " << endl
    //      << "  # atoms:    " << atoms.size() << endl
    //      << "  # points:   " << points.size() << endl
    //      << "  # to check: " << count << endl;
    if ((atoms.size() >= count) && (points.size() >= count)) {
      for (int i = count - 1; i >= 0; --i) {
        const mol::Atom &a(atoms[i]);
        const Point &p(points[i]);
        if ((a.x() != p[0]) || (a.y() != p[1]) || (a.z() != p[2])) {
          return false;
        }
      }
      return true;
    }
    return false;
  }

  void get_pointlist_info(const PointList &points, float &xmid, float &ymid,
                          float &zmid, float &width, float &height,
                          float &depth) {
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
        xmin = min(xmin, x);
        xmax = max(xmax, x);
        ymin = min(ymin, y);
        ymax = max(ymax, y);
        zmin = min(zmin, z);
        zmax = max(zmax, z);
      }

      xmid = xsum / points.size();
      ymid = ysum / points.size();
      zmid = zsum / points.size();
      width = xmax - xmin;
      height = ymax - ymin;
      depth = zmax - zmin;
    }
  }

  bool is_mean_centered(const PointList &points) {
    float xmid, ymid, zmid, w, h, d;
    get_pointlist_info(points, xmid, ymid, zmid, w, h, d);
    auto matcher = Catch::Matchers::WithinAbs(0.0, 0.0001);
    return matcher.match(xmid) && matcher.match(ymid) && matcher.match(zmid);
  }

  bool has_nonincreasing_extents(const PointList &points) {
    float xmid, ymid, zmid, w, h, d;
    get_pointlist_info(points, xmid, ymid, zmid, w, h, d);
    bool result = ((w >= h) && (h >= d) && (d > 0));
    return result;
  }

  float find_max_radius(const PointList &points) {
    float result = 0.0;
    for (const auto &point : points) {
      result = max(result, point.at(3));
    }
    return result;
  }

  bool is_non_null_transform(Transform &a) {
    // WEAK!
    int low_row = a.getlowbound(1);
    int high_row = a.gethighbound(1);
    int low_col = a.getlowbound(2);
    int high_col = a.gethighbound(2);
    bool result = (((high_row - low_row) == 2) && ((high_col - low_col) == 2));
    if (result) {
      result = false;
      for (int r = low_row; !result && (r <= high_row); r++) {
        ap::raw_vector<double> row_v = a.getrow(r, low_col, high_col);
        double *curr_col = row_v.GetData();
        for (int icol = low_col; !result && (icol <= high_col); ++icol) {
          result = (*curr_col != 0.0);
          curr_col++;
        }
      }
    }
    return result;
  }
};
} // namespace

TEST_CASE("mesaac::shape::AxisAligner", "[mesaac]") {
  // This setup is performed separately for each section.
  TestFixture fixture;
  std::unique_ptr<WBAxisAligner> aligner(fixture.new_aligner());
  mol::AtomVector atoms;
  PointList points;

  SECTION("Get atom coords from an empty vector of atoms") {
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE((size_t)0 == atoms.size());
    REQUIRE((size_t)0 == points.size());
    aligner->wb_get_atom_points(atoms, points, true);
    REQUIRE((size_t)0 == atoms.size());
    REQUIRE((size_t)0 == points.size());
  }

  SECTION("Get atom coords from a vector of heavy atoms") {
    unsigned int num_heavies;

    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE(atoms.size() > num_heavies);
    REQUIRE(num_heavies > 0);
    REQUIRE(fixture.coords_match(atoms, points, num_heavies));

    aligner->wb_get_atom_points(atoms, points, true);
    REQUIRE(fixture.coords_match(atoms, points, atoms.size()));
  }

  SECTION("Get the mean center of an empty list of points") {
    Point center;

    aligner->wb_get_mean_center(points, center);
    REQUIRE(center.size() == 3);
    REQUIRE(center[0] == 0.0f);
    REQUIRE(center[1] == 0.0f);
    REQUIRE(center[2] == 0.0f);
  }

  SECTION("Get the mean center of a set of heavy atoms") {
    unsigned int num_heavies;
    Point center;

    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    aligner->wb_get_mean_center(points, center);
    REQUIRE(center.size() == 3);

    // FRAGILE
    REQUIRE_THAT(center[0], Catch::Matchers::WithinAbs(24.1596, 0.0001));
    REQUIRE_THAT(center[1], Catch::Matchers::WithinAbs(22.0163, 0.0001));
    REQUIRE_THAT(center[2], Catch::Matchers::WithinAbs(15.9366, 0.0001));
  }

  SECTION("Translate an empty list of points so that its mean center lies at "
          "the origin") {
    REQUIRE(points.empty());
    aligner->wb_mean_center_points(points);
    // If execution reaches this point without crashing, the test passes.
    REQUIRE(points.empty());
  }

  SECTION("Translate a vector of heavy atoms so its mean center lies at the "
          "origin") {
    unsigned int num_heavies;
    Point center;

    // TODO:  create a set of atoms w. known positions and
    // easily verified extents.
    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE((size_t)num_heavies == points.size());
    aligner->wb_mean_center_points(points);
    REQUIRE((size_t)num_heavies == points.size());
    REQUIRE(fixture.is_mean_centered(points));

    float xmid, ymid, zmid, width, height, depth;
    fixture.get_pointlist_info(points, xmid, ymid, zmid, width, height, depth);
    REQUIRE_THAT(width, Catch::Matchers::WithinAbs(9.9782, 0.00001));
    REQUIRE_THAT(height, Catch::Matchers::WithinAbs(4.4967, 0.00001));
    REQUIRE_THAT(depth, Catch::Matchers::WithinAbs(6.8714, 0.00001));
  }

  SECTION("Get mean-centered cloud -- empty") {
    PointList cloud;
    REQUIRE(points.empty());
    aligner->wb_get_mean_centered_cloud(points, cloud);
    // If we get here without crashing, we win.
    REQUIRE(cloud.empty());
  }

  SECTION("Get mean-centered cloud -- non-empty") {
    unsigned int num_heavies;
    PointList cloud;
    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE(!points.empty());
    REQUIRE(points.size() == num_heavies);
    aligner->wb_mean_center_points(points);
    REQUIRE(points.size() == num_heavies);

    aligner->wb_get_mean_centered_cloud(points, cloud);
    REQUIRE(!cloud.empty());

    float xmid, ymid, zmid, pwidth, pheight, pdepth;
    fixture.get_pointlist_info(points, xmid, ymid, zmid, pwidth, pheight,
                               pdepth);
    REQUIRE(fixture.is_mean_centered(points));
    REQUIRE_THAT(pwidth, Catch::Matchers::WithinAbs(9.9782, 0.00001));
    REQUIRE_THAT(pheight, Catch::Matchers::WithinAbs(4.4967, 0.00001));
    REQUIRE_THAT(pdepth, Catch::Matchers::WithinAbs(6.8714, 0.00001));

    float cwidth, cheight, cdepth;
    fixture.get_pointlist_info(cloud, xmid, ymid, zmid, cwidth, cheight,
                               cdepth);
    float dmax = 2.0 * fixture.find_max_radius(points);

    // Hardwired badness: max radius of all atoms in create_sample_atoms should
    // be that of sulfur.
    REQUIRE_THAT(dmax, Catch::Matchers::WithinAbs(
                           mesaac::mol::get_symbol_radius("S") * 2, 0.00001));

    float dwidth = cwidth - pwidth, dheight = cheight - pheight,
          ddepth = cdepth - pdepth;

    REQUIRE(dwidth > 0);
    REQUIRE(dwidth <= dmax);
    REQUIRE(dheight > 0);
    REQUIRE(dheight <= dmax);
    REQUIRE(ddepth > 0);
    REQUIRE(ddepth <= dmax);
  }

  SECTION("Find axis-align transform") {
    PointList cloud;
    unsigned int num_heavies;

    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    aligner->wb_mean_center_points(points);
    aligner->wb_get_mean_centered_cloud(points, cloud);

    // Not sure how to test this.  Just confirm it's a 3x3 matrix
    // with non-empty cells?
    Transform transform;
    REQUIRE(!fixture.is_non_null_transform(transform));
    aligner->wb_find_axis_align_transform(cloud, transform);
    REQUIRE(fixture.is_non_null_transform(transform));
  }

  SECTION("Untranslate points") {
    Point offset{1.0, 2.0, 3.0};

    PointList points;

    // Ensure proper handling of empty lists:
    aligner->wb_untranslate_points(points, offset);
    REQUIRE(points.empty());

    unsigned int i;
    const unsigned int i_max = 10;
    for (i = 0; i != i_max; i++) {
      points.push_back({i + 1.0f, i + 2.0f, i + 3.0f});
    }

    aligner->wb_untranslate_points(points, offset);
    for (i = 0; i != i_max; i++) {
      float f(i);
      Point &p(points[i]);
      REQUIRE_THAT(p.at(0), Catch::Matchers::WithinAbs(f, 0.00001));
      REQUIRE_THAT(p.at(1), Catch::Matchers::WithinAbs(f, 0.00001));
      REQUIRE_THAT(p.at(2), Catch::Matchers::WithinAbs(f, 0.00001));
    }
  }

  SECTION("Transform points") {
    PointList cloud;
    unsigned int num_heavies;
    Transform transform;

    // Ensure no crash on empty cloud:
    REQUIRE_THROWS_AS(aligner->wb_find_axis_align_transform(cloud, transform),
                      invalid_argument);

    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    aligner->wb_mean_center_points(points);
    aligner->wb_get_mean_centered_cloud(points, cloud);

    REQUIRE(!fixture.is_non_null_transform(transform));
    aligner->wb_find_axis_align_transform(cloud, transform);
    REQUIRE(fixture.is_non_null_transform(transform));

    // Verify no crash on an empty point list.
    points.clear();
    aligner->wb_transform_points(points, transform);
    REQUIRE(points.empty());

    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE(!fixture.has_nonincreasing_extents(points));

    aligner->wb_transform_points(points, transform);
    REQUIRE(points.size() == num_heavies);

    // Transform should merely rotate the points -- no mean-centering.
    REQUIRE(fixture.has_nonincreasing_extents(points));
  }

  SECTION("Update atom coords") {
    PointList cloud;
    unsigned int num_heavies;

    // Ensure correct copying of transformed point coords to
    // corresponding atom coords.
    fixture.create_sample_atoms(atoms, num_heavies);

    // Point and atom lists of different size?  This should fail.
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE_THROWS_AS(aligner->wb_update_atom_coords(atoms, points),
                      std::length_error);

    // Moronic, but maybe adequate, test: superpose all atoms.
    aligner->wb_get_atom_points(atoms, points, true);
    Point offset{10.0, -50.0, 0.0};
    PointList::iterator i;
    for (i = points.begin(); i != points.end(); ++i) {
      Point &p(*i);
      p.at(0) = offset[0];
      p.at(1) = offset[1];
      p.at(2) = offset[2];
    }
    aligner->wb_update_atom_coords(atoms, points);

    for (const auto &atom : atoms) {
      REQUIRE_THAT(offset[0], Catch::Matchers::WithinAbs(atom.x(), 0.00001));
      REQUIRE_THAT(offset[1], Catch::Matchers::WithinAbs(atom.y(), 0.00001));
      REQUIRE_THAT(offset[2], Catch::Matchers::WithinAbs(atom.z(), 0.00001));
    }
  }

  SECTION("Align to axes") {
    // No crash on empty:
    aligner->align_to_axes(atoms);
    REQUIRE(atoms.empty());

    PointList points, cloud;
    unsigned int num_heavies;

    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE(!fixture.is_mean_centered(points));
    REQUIRE(!fixture.has_nonincreasing_extents(points));

    // TODO:  Check that the hydrogens are also transformed.
    aligner->align_to_axes(atoms);
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE(fixture.is_mean_centered(points));
    REQUIRE(fixture.has_nonincreasing_extents(points));
  }

  SECTION("Align mol to axes") {
    mol::Mol mol;

    // No crash on empty:
    aligner->align_to_axes(mol);
    REQUIRE(mol.atoms().empty());

    PointList points, cloud;
    unsigned int num_heavies;

    fixture.create_sample_mol(mol, num_heavies);
    aligner->wb_get_atom_points(mol.atoms(), points, false);
    REQUIRE(!fixture.is_mean_centered(points));
    REQUIRE(!fixture.has_nonincreasing_extents(points));

    // TODO:  Check that the hydrogens are also transformed.
    aligner->align_to_axes(mol);
    aligner->wb_get_atom_points(mol.atoms(), points, false);
    REQUIRE(fixture.is_mean_centered(points));
    REQUIRE(fixture.has_nonincreasing_extents(points));
  }

  SECTION("Align to axes - mol only") {
    std::unique_ptr<WBAxisAligner> aligner(fixture.new_aligner_ac_only());
    mol::Mol mol;

    // No crash on empty:
    aligner->align_to_axes(mol);
    REQUIRE(mol.atoms().empty());

    PointList points, cloud;
    unsigned int num_heavies;

    fixture.create_sample_mol(mol, num_heavies);
    aligner->wb_get_atom_points(mol.atoms(), points, false);
    REQUIRE(!fixture.is_mean_centered(points));
    REQUIRE(!fixture.has_nonincreasing_extents(points));

    // TODO:  Check that the hydrogens are also transformed.
    // TODO:  Check that align_to_axes with atom-centers-only produces
    // different results than without atom-centers-only.
    aligner->align_to_axes(mol);
    aligner->wb_get_atom_points(mol.atoms(), points, false);
    REQUIRE(fixture.is_mean_centered(points));
    REQUIRE(fixture.has_nonincreasing_extents(points));
  }

  SECTION("Align hydrogens") {
    // This is pretty naive: lay out some atoms in a line.
    // Confirm that the line, including its hydrogens, gets mapped onto
    // the X axis.

    mol::Mol mol;
    float x, y, z, w, h, d;

    fixture.add_atom(mol, "C", 0.0, 0.0, 0.0); // Stay inside the point cloud
    fixture.add_atom(mol, "H", 0.0, 0.0, 1.0);
    fixture.add_atom(mol, "C", 0.0, 0.0, 2.0);
    fixture.add_atom(mol, "H", 0.0, 0.0, 3.0);
    fixture.add_atom(mol, "C", 0.0, 0.0, 4.0);
    fixture.add_atom(mol, "H", 0.0, 0.0, 5.0);
    fixture.add_atom(mol, "C", 0.0, 0.0, 6.0);
    fixture.add_atom(mol, "H", 0.0, 0.0, 7.0);

    aligner->wb_get_atom_points(mol.atoms(), points, true);
    fixture.get_pointlist_info(points, x, y, z, w, h, d);

    REQUIRE_THAT(w, Catch::Matchers::WithinAbs(0.0f, 0.00001));
    REQUIRE_THAT(h, Catch::Matchers::WithinAbs(0.0f, 0.00001));
    REQUIRE_THAT(d, Catch::Matchers::WithinAbs(7.0f, 0.00001));

    aligner->align_to_axes(mol);
    aligner->wb_get_atom_points(mol.atoms(), points, true);

    fixture.get_pointlist_info(points, x, y, z, w, h, d);

    // The slope from point to point should be consistent.
    const float dx_dy = w / h;
    const float dx_dz = w / d;
    const float dy_dz = h / d;
    const mol::AtomVector &atoms(mol.atoms());
    mol::AtomVector::const_iterator i;
    mol::AtomVector::const_iterator iprev = atoms.end();
    for (i = atoms.begin(); i != atoms.end(); ++i) {
      if (iprev != atoms.end()) {
        const mol::Atom &prev(*iprev);
        const mol::Atom &curr(*i);
        // Not the best test -- dunno the expected direction:
        float dx = ::fabs(curr.x() - prev.x());
        float dy = ::fabs(curr.y() - prev.y());
        float dz = ::fabs(curr.z() - prev.z());
        REQUIRE_THAT(dx_dy, Catch::Matchers::WithinRel(dx / dy, 0.00014f));
        REQUIRE_THAT(dx_dz, Catch::Matchers::WithinRel(dx / dz, 0.00014f));
        REQUIRE_THAT(dy_dz, Catch::Matchers::WithinRel(dy / dz, 0.00014f));
      }
      iprev = i;
    }

    // The axis alignment is not perfect.
    REQUIRE_THAT(w, Catch::Matchers::WithinAbs(7.0f, 0.001f));
    REQUIRE_THAT(h, Catch::Matchers::WithinAbs(0.0f, 0.0125f));
    REQUIRE_THAT(d, Catch::Matchers::WithinAbs(0.0f, 0.05f));
  }

  SECTION("Overlapping spherules") {
    // This is a simple test which demonstrates that,
    // if two spherules overlap, their corresponding cloud points
    // will not be double-counted.

    const Point atom({0, 0, 0, 1.7});

    PointList mol1, mol2;
    mol1.push_back(atom);
    // Duplicates, superposed!
    mol2.push_back(atom);
    mol2.push_back(atom);

    PointList contained1, contained2;
    aligner->wb_get_mean_centered_cloud(mol1, contained1);
    aligner->wb_get_mean_centered_cloud(mol2, contained2);

    REQUIRE(contained1 == contained2);
    REQUIRE(!contained1.empty());
  }
}

namespace {
int benchmark_align_to_axes(const TestFixture &fixture,
                            std::shared_ptr<WBAxisAligner> aligner) {
  mol::Mol mol;
  PointList points, cloud;
  unsigned int num_heavies;

  fixture.create_sample_mol(mol, num_heavies);
  aligner->wb_get_atom_points(mol.atoms(), points, false);

  // TODO:  Check that the hydrogens are also transformed.
  aligner->align_to_axes(mol);
  aligner->wb_get_atom_points(mol.atoms(), points, false);
  return 0;
}

TEST_CASE("Benchmark mesaac::shape::AxisAligner",
          "[mesaac][mesaac_benchmark]") {
  TestFixture fixture;

  BENCHMARK_ADVANCED("Point cloud alignment")(
      Catch::Benchmark::Chronometer meter) {
    std::shared_ptr<WBAxisAligner> aligner(fixture.new_aligner());
    meter.measure([fixture, aligner] {
      return benchmark_align_to_axes(fixture, aligner);
    });
  };

  BENCHMARK_ADVANCED("Atom center alignment")(
      Catch::Benchmark::Chronometer meter) {
    std::shared_ptr<WBAxisAligner> ac_aligner(fixture.new_aligner_ac_only());
    meter.measure([fixture, ac_aligner] {
      return benchmark_align_to_axes(fixture, ac_aligner);
    });
  };
}

} // namespace
} // namespace mesaac::shape