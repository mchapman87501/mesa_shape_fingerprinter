// Find the shape volumes of conformers using quasi-Monte Carlo
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.
//
// This program takes as input an sd file of conformers, a file of Hammersly
// points arrayed in a sphere of a size into which small molecules
// will fit.

// Steps For all conformers:
// 1.  Mean center conformer.
//
// 2.  Calculate Hammersley points sphere volume.
//
// 3.  Find Hammersley points in atom defined volumes of conformer and count
// them.
//
// 4.  Multiply the Hammersley points sphere volume by the count of the points
// in
//     atom volumes of the conformer divided by the total number of Hammersley
//     points. Return this value as it represents the volume of the conformer.
//
// Note: Conformers of the same molecule will not differ in volume by more than
// 1 or 2% In general, there is no need to calculate multiple conformers for a
// single molecule.

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <numbers>
#include <string>
#include <vector>

#include "mesaac_mol/io/sdreader.hpp"
#include "mesaac_mol/mol.hpp"

using namespace std;
using namespace mesaac;

static void show_usage(const char *exename, string msg = "") {
  const std::filesystem::path prog_path(exename);
  const std::string prog_name(prog_path.stem());
  cerr << "Usage: " << prog_name << " sd_file hamms_sphere_file " << endl
       << "       sphere_radius atom_scale" << endl
       << "Print the volumes of a set of conformers." << endl
       << endl
       << "sd_file           - a file of conformers in SD format, with 3D "
          "coordinates"
       << endl
       << "hamms_sphere_file - a file containing 3D Hammersley sphere points, "
          "one point"
       << endl
       << "                    per line with space-separated coordinates, for "
          "principal"
       << endl
       << "                    axes generation via SVD" << endl
       << "sphere_radius     - radius of Hammersley sphere points (see " << endl
       << "                    hammersley_spheroid options)" << endl
       << "atom_scale        - factor by which to increase/decrease atom "
          "radii, relative"
       << endl
       << "                    to their van der Waals radii" << endl;
  if (!msg.empty()) {
    cerr << endl << msg << endl;
  }
  exit(1);
}

typedef vector<float> FloatVector;
typedef vector<FloatVector> CoordsList;

void read_sphere_points(const string &sdf_pathname, CoordsList &sphere_points) {
  sphere_points.clear();
  ifstream inf(sdf_pathname.c_str());
  if (!inf) {
    cerr << "Could not open sphere points file '" << sdf_pathname
         << "' for reading." << endl;
    exit(1);
  }

  float coord;
  while (inf >> coord) {
    FloatVector point;
    point.push_back(coord);
    inf >> coord;
    point.push_back(coord);
    inf >> coord;
    point.push_back(coord);
    sphere_points.push_back(point);
  }
  inf.close();
}

bool sphere_intersects_compound(const FloatVector &sphere_point,
                                const CoordsList &compound_coords,
                                const float epsilon) {
  for (const auto &point : compound_coords) {
    // boundarizing limit
    const float curr_atom_radius(point[3]);
    const float max_boundary = curr_atom_radius * epsilon;
    // Only bother to calc distance if inside boundary
    if (fabs(sphere_point[0] - point[0]) <= max_boundary) {
      const float dx = (sphere_point[0] - point[0]),
                  dy = (sphere_point[1] - point[1]),
                  dz = (sphere_point[2] - point[2]),
                  distance = ::sqrtf((dx * dx) + (dy * dy) + (dz * dz));

      if (distance <= max_boundary) {
        // Count each sphere point only once.
        return true;
      }
    }
  }
  return false;
}

unsigned int count_mol_sphere_points(const CoordsList &hamms_sphere_coords,
                                     const CoordsList &compound_coords,
                                     const float epsilon) {
  // Calculate volume fraction that the conformer takes up inside the sphere
  unsigned int count = 0.0;
  for (const auto &sphere_point : hamms_sphere_coords) {
    if (sphere_intersects_compound(sphere_point, compound_coords, epsilon)) {
      count++;
    }
  }
  return count;
}

int main(int argc, const char **const argv) {
  // Point and coordinate variables and objects
  CoordsList hamms_sphere_coords;

  if (argc != 5) {
    show_usage(argv[0], "Wrong number of arguments.");
  }
  // TODO  Should size be fixed?  Should volume be fixed?

  const std::filesystem::path sdf_pathname(argv[1]);
  const std::filesystem::path sphere_points_pathname(argv[2]);
  const float radius = std::stof(argv[3]);
  const float epsilon = std::stof(argv[4]); // AKA atom_scale

  read_sphere_points(sphere_points_pathname, hamms_sphere_coords);
  const unsigned int hamms_sphere_seq_size = hamms_sphere_coords.size();
  const float volume =
      std::numbers::pi * (4.0 / 3.0) * radius * radius * radius;

  ifstream sdf_inf(sdf_pathname.c_str());
  if (!sdf_inf) {
    cerr << "Could not open SD file " << sdf_pathname << " for reading."
         << endl;
    exit(1);
  }
  mol::SDReader reader(sdf_inf);

  mol::Mol mol;
  while (reader.read(mol)) {
    CoordsList compound_coords;
    float x_sum = 0.0, y_sum = 0.0, z_sum = 0.0;

    for (const auto &atom : mol.atoms()) {
      if (!atom.is_hydrogen()) {
        const auto &pos(atom.pos());
        const float x(pos.x()), y(pos.y()), z(pos.z()), r(atom.radius());
        compound_coords.push_back({x, y, z, r});
        x_sum += x;
        y_sum += y;
        z_sum += z;
      }
    }

    // Mean center points.
    const unsigned int compound_coords_size = compound_coords.size();
    const float x_mean = x_sum / compound_coords_size;
    const float y_mean = y_sum / compound_coords_size;
    const float z_mean = z_sum / compound_coords_size;
    for (auto &coords : compound_coords) {
      coords[0] -= x_mean;
      coords[1] -= y_mean;
      coords[2] -= z_mean;
    }

    // Calculate volume fraction that the conformer takes up inside the sphere
    const unsigned int count =
        count_mol_sphere_points(hamms_sphere_coords, compound_coords, epsilon);

    // Output volumes in cubic Angstroms.  Note, Blobby (space filling)
    // fudge factor epsilon will alter volumes.

    // TODO what should epsilon be to approximate generally accepted volume
    // calculation?
    cout << ((volume * count) / hamms_sphere_seq_size) << endl;
  }
  return 0;
}
