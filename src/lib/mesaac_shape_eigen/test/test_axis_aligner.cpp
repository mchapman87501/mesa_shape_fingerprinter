// Unit tests for mol_aligner.
// Copyright (c) 2010 Mesa Analytics & Computing, Inc.  All rights reserved
//

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include <fstream>

#include "mesaac_mol/element_info.hpp"
#include "mesaac_shape_eigen/axis_aligner.hpp"

using namespace std;

namespace mesaac::shape_eigen {

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

struct TestFixture {
  void get_point_means(const PointList &points, float &x, float &y, float &z) {
    x = y = z = 0.0;
    if (points.size()) {
      PointList::const_iterator i;
      float xsum = 0, ysum = 0, zsum = 0;
      for (i = points.begin(); i != points.end(); ++i) {
        const Point &p(*i);
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
      PointList::const_iterator i;
      float xmin = 0, ymin = 0, zmin = 0, xmax = 0, ymax = 0, zmax = 0;
      for (i = points.begin(); i != points.end(); ++i) {
        const Point &p(*i);
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

  void read_test_points(string pathname, PointList &points) {
    const string test_data_dir(TEST_DATA_DIR);
    const string data_dir(test_data_dir + "/hammersley/");
    const string full_path(data_dir + pathname);
    points.clear();
    ifstream inf(full_path.c_str());
    if (!inf) {
      ostringstream msg;
      msg << "Could not open " << pathname << " for reading." << endl;
      FAIL(msg.str());
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

  mol::Atom atom(string symbol, float x, float y, float z) const {
    const unsigned char atomic_num(mol::get_atomic_num(symbol));
    return {{atomic_num, {x, y, z}}};
  } // namespace shape_eigen

  void create_sample_mol(mol::Mol &mol, unsigned int &num_heavies) const {
    // Coordinates taken from first cox2_3d conformer.
    mol::AtomVector atoms{
        atom("C", 27.7051, 22.0403, 17.0243),
        atom("N", 26.4399, 22.0976, 16.4318),
        atom("C", 25.5381, 21.4424, 17.2831),
        atom("C", 26.2525, 20.9753, 18.3748),
        atom("C", 27.5943, 21.3608, 18.2218),
        atom("C", 24.0821, 21.3670, 17.1082),
        atom("C", 26.1324, 22.6824, 15.1634),
        atom("C", 23.4105, 22.2668, 16.2675),
        atom("C", 22.0220, 22.2007, 16.1197),
        atom("C", 21.2976, 21.2409, 16.8307),
        atom("C", 21.9509, 20.3402, 17.6750),
        atom("C", 23.3399, 20.4115, 17.8175),
        atom("C", 26.3695, 24.0457, 14.9358),
        atom("C", 26.0627, 24.6119, 13.6959),
        atom("C", 25.5236, 23.8179, 12.6910),
        atom("C", 25.2821, 22.4660, 12.9010),
        atom("C", 25.5848, 21.8942, 14.1391),
        atom("F", 25.2311, 24.3643, 11.5034),
        atom("C", 28.9655, 22.5443, 16.4025),
        atom("S", 19.5328, 21.1457, 16.6315),
        atom("O", 19.0413, 22.4851, 16.3229),
        atom("O", 18.9873, 20.4577, 17.7975),
        atom("C", 19.3258, 20.1152, 15.2035),
        atom("H", 25.8309, 20.4354, 19.2156),
        atom("H", 28.4100, 21.1477, 18.9041),
        atom("H", 23.9630, 23.0298, 15.7210),
        atom("H", 21.5209, 22.9035, 15.4575),
        atom("H", 21.3956, 19.5863, 18.2287),
        atom("H", 23.8404, 19.7079, 18.4815),
        atom("H", 26.7480, 24.6961, 15.7203),
        atom("H", 26.2341, 25.6696, 13.5167),
        atom("H", 24.8658, 21.8592, 12.1019),
        atom("H", 25.4187, 20.8273, 14.2701),
        atom("H", 29.8338, 22.0387, 16.8401),
        atom("H", 29.0870, 23.6157, 16.5858),
        atom("H", 28.9947, 22.3510, 15.3261),
        atom("H", 18.2555, 20.0118, 15.0126),
        atom("H", 19.7633, 19.1356, 15.4046),
        atom("H", 19.8115, 20.5898, 14.3489),
    };
    mol = mol::Mol({.atoms = atoms});
    num_heavies = 23;
  }

  void create_sample_atoms(mol::AtomVector &atoms, unsigned int &num_heavies) {
    mol::Mol mol;

    create_sample_mol(mol, num_heavies);
    // Take care to deep-copy all of the atom pointers.
    atoms.clear();
    const mol::AtomVector &src(mol.atoms());
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
        const mol::Atom &atom(atoms[i]);
        const Point &p(points[i]);
        if ((atom.x() != p[0]) || (atom.y() != p[1]) || (atom.z() != p[2])) {
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

  bool is_non_null_transform(Transform &atom) { return !atom.isZero(); }
};

TEST_CASE("mesaac::shape_eigen::AxisAligner", "[mesaac]") {
  TestFixture fixture;
  std::unique_ptr<WBAxisAligner> aligner(fixture.new_aligner());
  mol::AtomVector atoms;
  PointList points;

  SECTION("Get atom coords from an empty vector of atoms") {

    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE(atoms.empty());
    REQUIRE(points.empty());
    aligner->wb_get_atom_points(atoms, points, true);
    REQUIRE(atoms.empty());
    REQUIRE(points.empty());
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
    REQUIRE(center.size() == (size_t)3);
    REQUIRE(center[0] == 0.0f);
    REQUIRE(center[1] == 0.0f);
    REQUIRE(center[2] == 0.0f);
  }

  SECTION("Get atom coords from a vector of heavy atoms") {
    unsigned int num_heavies;
    Point center;

    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    aligner->wb_get_mean_center(points, center);
    REQUIRE(center.size() == (size_t)3);
    // FRAGILE
    REQUIRE_THAT(24.1596, Catch::Matchers::WithinAbs(center[0], 0.0001));
    REQUIRE_THAT(22.0163, Catch::Matchers::WithinAbs(center[1], 0.0001));
    REQUIRE_THAT(15.9366, Catch::Matchers::WithinAbs(center[2], 0.0001));
  }

  SECTION("Get the mean center of an empty list of points") {
    REQUIRE(points.empty());
    aligner->wb_mean_center_points(points);
    // If we make it this far, we pass.
    REQUIRE(points.empty());
  }

  SECTION("Get the mean center of a set of heavy atoms") {
    unsigned int num_heavies;
    Point center;

    // TODO:  create a set of atoms w. known positions and
    // easily verified extents.
    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE(points.size() == (size_t)num_heavies);
    aligner->wb_mean_center_points(points);
    REQUIRE(points.size() == (size_t)num_heavies);
    REQUIRE(fixture.is_mean_centered(points));

    float xmid, ymid, zmid, width, height, depth;
    fixture.get_pointlist_info(points, xmid, ymid, zmid, width, height, depth);
    REQUIRE_THAT(9.9782, Catch::Matchers::WithinAbs(width, 0.00001));
    REQUIRE_THAT(4.4967, Catch::Matchers::WithinAbs(height, 0.00001));
    REQUIRE_THAT(6.8714, Catch::Matchers::WithinAbs(depth, 0.00001));
  }

  SECTION("Get mean-centered cloud -- empty") {
    PointList cloud;
    REQUIRE(points.empty());
    aligner->wb_get_mean_centered_cloud(points, cloud);
    // If we get here without crashing, we win.
    REQUIRE(cloud.empty());
  }

  SECTION("Get mean-centered cloud -- non-empty") {
    PointList cloud;
    unsigned int num_heavies;

    fixture.create_sample_atoms(atoms, num_heavies);
    aligner->wb_get_atom_points(atoms, points, false);
    REQUIRE(points.size() > 0);
    REQUIRE(points.size() == (size_t)num_heavies);
    aligner->wb_mean_center_points(points);
    REQUIRE(points.size() == (size_t)num_heavies);
    aligner->wb_get_mean_centered_cloud(points, cloud);
    REQUIRE(cloud.size() > 0);

    float xmid, ymid, zmid, pwidth, pheight, pdepth;
    fixture.get_pointlist_info(points, xmid, ymid, zmid, pwidth, pheight,
                               pdepth);
    REQUIRE(fixture.is_mean_centered(points));
    REQUIRE_THAT(9.9782, Catch::Matchers::WithinAbs(pwidth, 0.00001));
    REQUIRE_THAT(4.4967, Catch::Matchers::WithinAbs(pheight, 0.00001));
    REQUIRE_THAT(6.8714, Catch::Matchers::WithinAbs(pdepth, 0.00001));

    float cwidth, cheight, cdepth;
    fixture.get_pointlist_info(cloud, xmid, ymid, zmid, cwidth, cheight,
                               cdepth);
    float dmax = 2.0 * fixture.find_max_radius(points);
    // Bench-check:  max radius should be 1.8, for sulfur.
    REQUIRE(dmax == 3.6f);

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
    Transform transform = Transform::Zero();
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
      REQUIRE(p.at(0) == f);
      REQUIRE(p.at(1) == f);
      REQUIRE(p.at(2) == f);
    }
  }

  SECTION("Transform points") {
    PointList cloud;
    unsigned int num_heavies;
    Transform transform = Transform::Zero();

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
    REQUIRE(points.size() == (size_t)num_heavies);

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
    Point offset({10.0, -50.0, 0.0});
    for (auto &p : points) {
      p.at(0) = offset[0];
      p.at(1) = offset[1];
      p.at(2) = offset[2];
    }
    aligner->wb_update_atom_coords(atoms, points);

    for (const auto &atom : atoms) {
      REQUIRE(atom.x() == offset[0]);
      REQUIRE(atom.y() == offset[1]);
      REQUIRE(atom.z() == offset[2]);
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

  SECTION("Point cloud alignment") {
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
    float x, y, z, w, h, d;

    mol::Mol mol(
        {.atoms = {
             fixture.atom("C", 0.0, 0.0, 0.0), // Stay inside the point cloud
             fixture.atom("H", 0.0, 0.0, 1.0),
             fixture.atom("C", 0.0, 0.0, 2.0),
             fixture.atom("H", 0.0, 0.0, 3.0),
             fixture.atom("C", 0.0, 0.0, 4.0),
             fixture.atom("H", 0.0, 0.0, 5.0),
             fixture.atom("C", 0.0, 0.0, 6.0),
             fixture.atom("H", 0.0, 0.0, 7.0),
         }});

    aligner->wb_get_atom_points(mol.atoms(), points, true);
    fixture.get_pointlist_info(points, x, y, z, w, h, d);
    REQUIRE(w == 0.0f);
    REQUIRE(h == 0.0f);
    REQUIRE(d == 7.0f);

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
        REQUIRE_THAT(dx_dy, Catch::Matchers::WithinAbs(dx / dy, 0.00025));
        REQUIRE_THAT(dx_dz, Catch::Matchers::WithinAbs(dx / dz, 0.00025));
        REQUIRE_THAT(dy_dz, Catch::Matchers::WithinAbs(dy / dz, 0.00025));
      }
      iprev = i;
    }

    // The axis alignment is not perfect.
    REQUIRE_THAT(7.0, Catch::Matchers::WithinAbs(w, 0.001));
    REQUIRE_THAT(0.0, Catch::Matchers::WithinAbs(h, 0.0125));
    REQUIRE_THAT(0.0, Catch::Matchers::WithinAbs(d, 0.05));
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
    REQUIRE(contained1.size() > 0);
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

} // namespace

TEST_CASE("mesaac::shape_eigen::AxisAligner Benchmarks",
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
} // namespace mesaac::shape_eigen
