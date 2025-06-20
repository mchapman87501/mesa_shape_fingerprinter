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
// 3.  Find Hammersley points in atom defined volumes of conformer and count them.
//
// 4.  Multiply the Hammersley points sphere volume by the count of the points in 
//     atom volumes of the conformer divided by the total number of Hammersley points.
//     Return this value as it represents the volume of the conformer.
// 
// Note: Conformers of the same molecule will not differ in volume by more than 1 or 2%
// In general, there is no need to calculate multiple conformers for a single 
// molecule.

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <libgen.h>
#include "mesaac_mol.h"
#include "mesaac_common/mesaac_common.h"

using namespace std;
using namespace mesaac;

static void showUsage(char *exename, string msg="") {
    cerr << "Usage: " << basename(exename) << " sd_file hamms_sphere_file " << endl
         << "       sphere_radius atom_scale" << endl
         << "Print the volumes of of a set of conformers." << endl
         << endl
         << "sd_file           = a file of conformers in SD format, with 3D coordinates" << endl
         << "hamms_sphere_file = a file containing 3D Hammersley sphere points, one point" << endl
         << "                    per line with space-separated coordinates, for principal" << endl
         << "                    axes generation via SVD" << endl
         << "sphere_radius     = radius of Hammersley sphere points (see " << endl
         << "                    hammersley_spheroid options)" << endl
         << "atom_scale        = factor by which to increase/decrease atom radii, relative" << endl
         << "                    to their van der Waals radii" << endl
         ;
    if (msg.size()) {
        cerr << endl << msg << endl;
    }
    exit(1);
}

typedef vector< float > FloatVector;
typedef vector< FloatVector > CoordsList;

void readSpherePoints(string pathname, CoordsList& spherePoints) {
    spherePoints.clear();
    ifstream inf(pathname.c_str());
    if (!inf) {
        cerr << "Could not open sphere points file '" << pathname 
             << "' for reading." << endl;
        exit(1);
    }
    
    float coord;
    while(inf >> coord){
        FloatVector point;
        point.push_back(coord);
        inf >> coord;
        point.push_back(coord);
        inf >> coord;
        point.push_back(coord);
        spherePoints.push_back(point);
    }
    inf.close();
}

int main(int argc, char **argv){
  
  unsigned int i,k;

  //Point and coordinate variables and objects
  CoordsList HammsSphereCoords;

  //Measure variables

  mesaac::MesaacFeatures f;
  f.set(mesaac::MFE_Miscellaneous);
  mesaac::initCommon(f); // Validate license.

  if (argc != 5){
      showUsage(argv[0], "Wrong number of arguments.");
  }
  //ToDo:  Should size be fixed?  Should volume be fixed?

  string pathname(argv[1]);
  ifstream sdf_inf(pathname.c_str());
  if(!sdf_inf){
      cerr << "Could not open " << pathname << " for reading." << endl;
      exit(1);
  }

  readSpherePoints(argv[2], HammsSphereCoords);
  const unsigned int HammsSphereSequenceSize = HammsSphereCoords.size();
  const float radius = atof(argv[3]);
  const float volume = M_PI * (4.0 / 3.0) * radius * radius * radius;
  const float epsilon = atof(argv[4]);
  
  float x_mean,y_mean,z_mean;

  mol::SDReader reader(sdf_inf);
  mol::Mol m;
  while (reader.read(m)) {
      CoordsList CompoundCoords;
      unsigned int count = 0.0;
      float x_sum = 0.0,
            y_sum = 0.0,
            z_sum = 0.0;
      mol::AtomVector::const_iterator j,
          jEnd(m.atoms().end());
      
      for (j = m.atoms().begin(); j != jEnd; ++j) {
          mol::Atom *a(*j);
          
          float x(a->x()), y(a->y()), z(a->z()), r(a->radius());
          
          // Comment out '&& !a->is_hydrogen()' to include hydrogens in volume
          if (a && !a->is_hydrogen()) {
              FloatVector point;
              point.push_back(x);
              point.push_back(y);
              point.push_back(z);
              point.push_back(r);
              x_sum += x;
              y_sum += y;
              z_sum += z;
              CompoundCoords.push_back(point);
          }
      }

      //Mean center points.
      unsigned int CompoundCoordsSize = CompoundCoords.size();
      x_mean = x_sum / CompoundCoordsSize;
      y_mean = y_sum / CompoundCoordsSize;
      z_mean = z_sum / CompoundCoordsSize;
      //cerr << CompoundCoordsSize << endl;
      for (i = 0; i < CompoundCoordsSize; i++) {
          FloatVector& c(CompoundCoords[i]);
          c[0] -= x_mean;
          c[1] -= y_mean;
          c[2] -= z_mean;
      }

      //Calculate volume fraction that the conformer takes up inside the sphere
      for (i = 0; i < HammsSphereSequenceSize; i++) {
          const FloatVector& spherePoint(HammsSphereCoords[i]);
          for (k = 0; k < CompoundCoords.size(); k++){
              const FloatVector& point(CompoundCoords[k]);
              //boundarizing limit
              const float max_boundary = point[3] * epsilon;
              //Only bother to calc distance if inside boundary
              if (fabs(spherePoint[0] - point[0]) <= max_boundary) {
                  float dx = (spherePoint[0] - point[0]),
                        dy = (spherePoint[1] - point[1]),
                        dz = (spherePoint[2] - point[2]);
                  float distance = 
                      ::sqrtf((dx * dx) + (dy * dy) + (dz * dz));
              
                  if (distance <= max_boundary) {
                    count++;
                    break;
                  }
              }
          }
      }
      //Output volumes in cubic Angstroms.  Note, Blobby (space filling) fudge factor epsilon 
      //will alter volumes.  ToDo: what should epsilon be to approximate generally accepted 
      //volume calculation?
      //cerr << volume << " " << count << " " << HammsSphereSequenceSize << endl;
      cout << ((volume * count) / HammsSphereSequenceSize) << endl;
  }
  return 0;  
}
