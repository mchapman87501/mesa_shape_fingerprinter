// Shape Radius on mean centered, PCA rotated conformations
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.
//
// This program takes as input an sd file of conformers, then the small molecule
// is aligned to the principle components of the atom center points.

// Step
// 1.  Read in conformations
//
// 2.  Mean center conformations
//
// 3.  For each conf, output max x, y, z, atom radius, and radius BEFORE PC
//
// 4.  Calculate PC and rotate conformer to PC respective
//
// 5.  For each conf, output max x, y, z, atom radius, and radius AFTER PC
//
// 6.  Output max x, y, z, atom radius, and radius for all conformers

#include "mesaac_common/mesaac_common.h"
#include "mesaac_mol.h"
#include "svd.h"
#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <libgen.h>
#include <string>
#include <vector>

using namespace std;
using namespace mesaac;

string Version = "1.0";
string CreationDate = "April 30, 2009";

static void showBlurb(char *exename) {
  cerr << "Running " << basename(exename) << endl
       << "Source code Copyright (c) 2009" << endl
       << "Mesa Analytics & Computing, Inc." << endl
       << "Version number " << Version << " Creation Date: " << CreationDate
       << endl
       << "Expiration Date: " << mesaac::expirationDateStr() << endl;
}

static void showUsage(char *exename, string msg = "") {
  cerr
      << "Usage: " << basename(exename)
      << " sd_file hamms_sphere_file atom_scale" << endl
      << "Find the maximum extent of any conformer in sd_file with respect to "
      << endl
      << "the mean centered conformations." << endl
      << endl
      << "sd_file           = a file of conformers in SD format, with 3D "
         "coordinates"
      << endl
      << "hamms_sphere_file = a file containing 3D Hammersley sphere points, "
         "one point"
      << endl
      << "                    per line with space-separated coordinates, for "
         "principal"
      << endl
      << "                    axes generation via SVD" << endl
      << "atom_scale        = the amount, in the range [1.0..2.0], by which to "
      << endl
      << "                    increase atom radii for alignment" << endl;
  if (msg.size()) {
    cerr << endl << msg << endl;
  }
  exit(1);
}

typedef vector<float> FloatVector;
typedef vector<FloatVector> CoordsList;

void readSpherePoints(string pathname, CoordsList &spherePoints) {
  ifstream inf(pathname);
  if (!inf) {
    cerr << "Cannot open Hammersly sphere points file. Abort." << endl;
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
    spherePoints.push_back(point);

    point.clear();
  }
  inf.close();
}

void getMolCoords(mol::Mol &m, CoordsList &coords, float &x_sum, float &y_sum,
                  float &z_sum) {
  x_sum = 0.0;
  y_sum = 0.0;
  z_sum = 0.0;
  mol::AtomVector::const_iterator i;
  mol::AtomVector::const_iterator iEnd(m.atoms().end());
  for (i = m.atoms().begin(); i != iEnd; ++i) {
    mol::Atom *a(*i);

    // Omit Hydrogens with this if statement
    if (!a->is_hydrogen()) {
      float x(a->x()), y(a->y()), z(a->z()), r(a->radius());
      FloatVector p;

      p.push_back(x);
      p.push_back(y);
      p.push_back(z);
      p.push_back(r);
      // cerr << "R: " << r << endl;

      x_sum += x;
      y_sum += y;
      z_sum += z;
      coords.push_back(p);
    }
  }
}

void meanCenterMolCoords(CoordsList &coords, float x_sum, float y_sum,
                         float z_sum) {
  unsigned int numCoords = coords.size();
  float x_mean = x_sum / numCoords;
  float y_mean = y_sum / numCoords;
  float z_mean = z_sum / numCoords;

  for (unsigned int i = 0; i != numCoords; i++) {
    FloatVector &currCoord(coords[i]);
    currCoord[0] -= x_mean;
    currCoord[1] -= y_mean;
    currCoord[2] -= z_mean;
  }
}

int main(int argc, char **argv) {

  unsigned int i, k;
  unsigned int PCAPointSize = 0;
  unsigned int CompoundPointSize = 0;
  // char c;
  // Point and coordinate variables and objects
  vector<float> point;

  vector<vector<float>> CompoundCoords;

  vector<vector<float>> TmpCoords1;
  vector<vector<float>> TmpCoords2;

  vector<vector<vector<float>>> CompoundsCoords;
  vector<vector<float>> HammsSphereCoords; // quasi-random sampling points

  // diagnostic variables
  float max_x, max_y, max_z, max_atom_radius, max_radius;
  float x_sum, y_sum, z_sum;

  vector<float> x_extremes, y_extremes, z_extremes, atom_radius_extremes,
      radius_extremes;
  float max_boundary;

  // Measure variables
  double distance;

  // PCA data structures
  ap::real_2d_array X;
  ap::real_1d_array W;
  ap::real_2d_array U;
  ap::real_2d_array VT;

  mesaac::MesaacFeatures f;
  f.set(mesaac::MFE_FingerprinterModule);
  mesaac::initCommon(f);

  showBlurb(argv[0]);

  if (argc != 4) {
    showUsage(argv[0], "Wrong number of arguments");
  }

  float epsilon = atof(argv[3]);

  string pathname(argv[1]);
  ifstream sdf_inf(argv[1]);
  if (!sdf_inf) {
    cerr << "Could not open " << pathname << " for reading." << endl;
    exit(1);
  }

  // Open Hammersly points file
  readSpherePoints(argv[2], HammsSphereCoords);

  unsigned int HammsSphereSequenceSize = HammsSphereCoords.size();

  mol::Mol m;
  mol::SDReader reader(sdf_inf);
  while (reader.read(m)) {
    getMolCoords(m, CompoundCoords, x_sum, y_sum, z_sum);
    meanCenterMolCoords(CompoundCoords, x_sum, y_sum, z_sum);

    // cout << "BEFORE PCA: " << max_x << " " << max_y << " " << max_z << " " <<
    // max_atom_radius << " " << max_radius << endl;
    vector<vector<float>> PCACoords;
    // cerr << "Compoundcoords here " << CompoundCoords.size() << endl;
    PCACoords = TmpCoords1 = TmpCoords2 = CompoundCoords;
    for (i = 0; i < HammsSphereSequenceSize; i++) {
      for (k = 0; k < CompoundCoords.size(); k++) {
        // boundarizing limit
        max_boundary = CompoundCoords[k][3] * epsilon;
        // Only bother to calc distance if inside boundary
        if (fabs(HammsSphereCoords[i][0] - CompoundCoords[k][0]) <=
            max_boundary) {
          distance = (float)sqrt(
              (double)((HammsSphereCoords[i][0] - CompoundCoords[k][0]) *
                           (HammsSphereCoords[i][0] - CompoundCoords[k][0]) +
                       (HammsSphereCoords[i][1] - CompoundCoords[k][1]) *
                           (HammsSphereCoords[i][1] - CompoundCoords[k][1]) +
                       (HammsSphereCoords[i][2] - CompoundCoords[k][2]) *
                           (HammsSphereCoords[i][2] - CompoundCoords[k][2])));

          if ((float)distance <= max_boundary) { // ADD point to PCACoords
            PCACoords.push_back(HammsSphereCoords[i]);
            break;
          }
        }
      }
    }
    PCAPointSize = PCACoords.size();
    CompoundPointSize = CompoundCoords.size();
    X.setbounds(0, PCAPointSize - 1, 0, 2); // PCAPointSize-1);

    // Mean center PCA points
    x_sum = 0.0;
    y_sum = 0.0;
    z_sum = 0.0;
    for (i = 0; i < PCAPointSize; i++) {
      x_sum += PCACoords[i][0];
      y_sum += PCACoords[i][1];
      z_sum += PCACoords[i][2];
    }

    float x_mean = x_sum / (float)PCAPointSize;
    float y_mean = y_sum / (float)PCAPointSize;
    float z_mean = z_sum / (float)PCAPointSize;
    for (i = 0; i < PCAPointSize; i++) {
      PCACoords[i][0] -= x_mean;
      PCACoords[i][1] -= y_mean;
      PCACoords[i][2] -= z_mean;
    }

    // Fill arrays for PCA code
    for (i = 0; i < PCAPointSize; i++) {
      for (k = 0; k < 3; k++) {
        X(i, k) = (double)PCACoords[i][k];
      }
    }

    if (!rmatrixsvd(X, PCAPointSize, 3, 0, 1, 2, W, U, VT)) {
      cerr << "PCA failed.  Abort";
      exit(1);
    }
    x_sum = 0.0;
    y_sum = 0.0;
    z_sum = 0.0;
    for (i = 0; i < CompoundPointSize; i++) {
      x_sum += PCACoords[i][0];
      y_sum += PCACoords[i][1];
      z_sum += PCACoords[i][2];
    }

    x_mean = x_sum / (float)CompoundPointSize;
    y_mean = y_sum / (float)CompoundPointSize;
    z_mean = z_sum / (float)CompoundPointSize;
    for (i = 0; i < CompoundPointSize; i++) {
      PCACoords[i][0] -= x_mean;
      PCACoords[i][1] -= y_mean;
      PCACoords[i][2] -= z_mean;
    }

    for (i = 0; i < CompoundPointSize; i++) {
      for (k = 0; k < 3; k++) {
        CompoundCoords[i][k] = (VT(0, k) * PCACoords[i][0]) +
                               (VT(1, k) * PCACoords[i][1]) +
                               (VT(2, k) * PCACoords[i][2]);
      }
    }
    // Why are these being computed twice?  Or, why were the
    // previous values not used?
    max_x = 0.0;
    max_y = 0.0;
    max_z = 0.0;
    max_radius = 0.0;
    max_atom_radius = 0.0;

    for (i = 0; i < CompoundCoords.size(); i++) {
      float curr_atom_radius = CompoundCoords[i][3];
      float curr_x = fabs(CompoundCoords[i][0]);
      float curr_y = fabs(CompoundCoords[i][1]);
      float curr_z = fabs(CompoundCoords[i][2]);
      float curr_radius =
          sqrt((curr_x * curr_x) + (curr_y * curr_y) + (curr_z * curr_z));

      max_x = (max_x > curr_x) ? max_x : curr_x;
      max_y = (max_y > curr_y) ? max_y : curr_y;
      max_z = (max_z > curr_z) ? max_z : curr_z;
      max_atom_radius = (max_atom_radius > curr_atom_radius) ? max_atom_radius
                                                             : curr_atom_radius;
      max_radius = (max_radius > curr_radius) ? max_radius : curr_radius;
    }
    // cout << "AFTER PCA: " << max_x << " " << max_y << " " << max_z << " " <<
    // max_atom_radius << " " << max_radius << endl;
    x_extremes.push_back(max_x);
    y_extremes.push_back(max_y);
    z_extremes.push_back(max_z);
    atom_radius_extremes.push_back(max_atom_radius);
    radius_extremes.push_back(max_radius);
    PCACoords.erase(PCACoords.begin(), PCACoords.end());
    CompoundCoords.erase(CompoundCoords.begin(), CompoundCoords.end());
  }
  float atom_radius_tmp =
      *max_element(atom_radius_extremes.begin(), atom_radius_extremes.end());
  cout << "Max_x " << "Max_y " << "Max_z " << "Max_atom_radius "
       << "Max_radius_plus_max_atom_radius" << endl;

  cout << *max_element(x_extremes.begin(), x_extremes.end()) << " "
       << *max_element(y_extremes.begin(), y_extremes.end()) << " "
       << *max_element(z_extremes.begin(), z_extremes.end()) << " "
       << atom_radius_tmp << " "
       << *max_element(radius_extremes.begin(), radius_extremes.end()) +
              atom_radius_tmp
       << endl;

  return 0;
}
