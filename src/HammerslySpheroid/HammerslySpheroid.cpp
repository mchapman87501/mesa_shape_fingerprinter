// Hammersly Sequence generator.  
// Copyright (c) 2009 Mesa Analytics & Computing, Inc.  All rights reserved
// Author John MacCuish.  
// Given Dimension and number of points to return, return Hammersly points for that 
// Dimension.  The Hammersly Sequence of N dimensions is successive van der Corput 
// linear sequences, where the first dimension is the uniform sequence i/N,
// and where succeeding dimensions are van der Corput sequences generated with distinct
// prime bases.  The first dimension uses 2 as its prime number and is done
// in a separate for loop for performance.  The remaining dimensions make calls to
// the "baseFunction" to generate the van der Corput sequence for a specific prime
// base.  The benefits of quasi-random sequences of the Hammersly sequence begins to 
// break down at around 40 dimensions without some fixes (not implemented here).  
// Thus, the dimensions are capped at for now at 29, as this begins to bump up 
// a floating point exception.  Floating point numbers are used for speed.  Double
// precision should be implemented as an option for exactness.

#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <bitset>
#include <libgen.h>

#include "mesaac_common/mesaac_common.h"

using namespace std;
unsigned int p[28] = {3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,103,107,109};

void showUsage(char *exename, string msg="") {
    cerr << "Usage: " << basename(exename) << " sample_size a b c scale" << endl
         << "Generate sample_size points within a unit cube, and print those" << endl
         << "points which lie within an ellipsoid volume described by a, b, c, and scale." << endl
         << endl
         << "sample_size = total number of 3D points to generate" << endl
         << "a b c       = ellipsoid axis scale parameters (continuous values)" << endl
         << "scale       = extent of largest ellipsoid axis" << endl
         ;
    if (msg.size()) {
        cerr << endl << msg << endl;
    }
    exit(1);
}

vector<unsigned int> baseFunction(unsigned int prime, unsigned int number)
{
    vector<unsigned int> base_vector;
    unsigned int quotient  = number;

    while(quotient > 0){
	base_vector.push_back(quotient % prime);
	quotient /= prime;
    }
    return base_vector;
};

int main(int argc, char **argv){
  
  unsigned int i,j,twocount;
  vector<unsigned int> primes(p,p+28);
  vector<unsigned int> baseVector;
  unsigned int SampleSize;
  double sum;
  vector<float> a_Dimension;
  vector< vector<float> > AllDimensions;
  float a = 1.0;
  float b = 1.0;
  float c = 1.0;

  float scale = 10;
  
  mesaac::MesaacFeatures f;
  f.set(mesaac::MFE_Miscellaneous);
  mesaac::initCommon(f);

  
  //represent the integers i = 1,...,K as binary
  //multiple least significant bit by 1 plus its binary place reciprocal
  //e.g., random number x_i = 1111 = 1/16 + 1/8 + 1/4 + 1/2 = 0.9375
  //take the floor of (N * x_i) = random index into the array of length N
  //Store indices in a k length vector of ints.
  if (argc != 6) {
      showUsage(argv[0], "Wrong number of arguments");
  }

  if(3 > primes.size()+1){
      cerr << "Too many dimensions.  Current cap is at " << primes.size()+1 << endl;
      cerr << "Try again" << endl;
      exit(1);
  }

  SampleSize= atoi(argv[1]);
  a = atof(argv[2]);
  b = atof(argv[3]);
  c = atof(argv[4]);

  scale = atof(argv[5]); 

  //Store first dimension, the sequence i/N (SampleSize)
  for(i = 1; i<=SampleSize; i++){
      a_Dimension.push_back((float)i/float(SampleSize));
  }
  AllDimensions.push_back(a_Dimension);
  a_Dimension.clear();

  //van der Corput second dimension with 2 as the first prime.  Faster than 
  //the remaining prime dimensions

  for(i = 1; i<=SampleSize; i++){
    j=0;
    sum = 0.0;
    twocount = 1;
    bitset<32> i_int(i);
    while(twocount <= i){
      if(i_int[j]){
        sum += 1.0/(float)(twocount*2);
      }
      j++;
      twocount *= 2;
    }
    a_Dimension.push_back(sum);
  }

  AllDimensions.push_back(a_Dimension);
  a_Dimension.clear();

  //Remaining dimensions are successive primes 3, 5, 7, ... 
  unsigned int count;
  unsigned int k = 0;
  while(k < 3){
      unsigned int prime = primes[k];
      for(i = 1; i<=SampleSize; i++){
	  j=0;
	  sum = 0.0;
	  count = 1;
	  baseVector = baseFunction(prime, i); 
	  while(count <= i){
	      if(baseVector[j]){
		  sum += baseVector[j]/(float)(count*prime);
	      }
	      j++;
	      count *= prime;
	  }
	  a_Dimension.push_back(sum);
      }
      AllDimensions.push_back(a_Dimension);
      a_Dimension.clear();
      k++;
  }


  //Center at origin (0,0,0) and scale
  for(i=0;i<SampleSize;i++){
      for(j = 0;j < 3; j++){
	  AllDimensions[j][i] = scale*(1.0 - 2.0*AllDimensions[j][i]);
      }
  }


  for(i=0;i<SampleSize;i++){
      if(::sqrt(((AllDimensions[0][i])*(AllDimensions[0][i]))/a + 
	      ((AllDimensions[1][i])*(AllDimensions[1][i]))/b + 
	      ((AllDimensions[2][i])*(AllDimensions[2][i]))/c) < double(scale)){
	  cout << AllDimensions[0][i] << " " << AllDimensions[1][i] << " " << AllDimensions[2][i] << endl;
      }
  }
  return 0;
}
