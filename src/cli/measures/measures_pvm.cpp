#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <cstdlib>

#include "Globals.h"
#include "mesaac_measures/measures.h"

#include "mesaac_common/mesaac_common.h"

using namespace std;

string Version = "1.2";
string CreationDate = "May, 2004";

int main(int argc, char **argv){
  
  int i,j;
  int targetnumber_fingerprints = 0;
  int querynumber_fingerprints = 0;
  float SparseThreshold;
  float tmpmeasure;
  string fingerprint;
  vector<string> fingerprints;

  mesaac::MesaacFeatures f;
  f.set(mesaac::MFE_GroupingModule);
  mesaac::initCommon(f);

  cerr << "Running MeasuresPVM.  Source code Copyright (c) 2005 Mesa Analytics & Computing, LLC" << endl
       << "Version number " << Version << " Creation Date: " << CreationDate << endl
       << "Expiration Date: " << mesaac::expirationDateStr() << endl;

  if (argc != 4){
      cerr << "Usage: " << argv[0]
         << " target.fingerprintfile.txt query.fingerprintfile.txt sparsethreshold" << endl
         << "target file contains M < N target fingerprints ("-" will read from stdin)," << endl
         << "where the first M of N queries are in the query file" << endl
         << "query file contains N > M query fingerprints ('-' will read from stdin)" << endl
         << "sparsethreshold is in the range of (0,1)" << endl;
    exit(1);
  }
  
  // Input from either stdin or file
  string targetfilename = argv[1];
  ifstream ifs1;
  if(targetfilename != "-"){
    ifs1.open(argv[1]);
    if(!ifs1){
      cerr << "Cannot open target fingerprint file. Abort." << endl;
      exit(1);
    }
  }                       
  
  // Input from either stdin or file
  string queryfilename = argv[2];
  ifstream ifs2;
  if(queryfilename != "-"){
    ifs2.open(argv[1]);
    if(!ifs2){
      cerr << "Cannot open query fingerprint file. Abort." << endl;
      exit(1);
    }
  }

  //Read in fingerprints from fingerprint file. First prime read to get vector
  //size
  SparseThreshold = atof(argv[3]);
 
  //Create a list of fingerprints to store each bitstring in
  ArrayBitVectors TargetFingerPrints;
  ArrayBitVectors QueryFingerPrints;

  //Length of fingerprint
  unsigned int vector_size;

  if(targetfilename == "-"){
    //Read in fingerprint from standard input
    cin >> fingerprint;
    //Determine the length of the first fingerprint
    vector_size = fingerprint.size();
    //Assign the fingerprint to a bitstring (bitset)
    BitVector aprint(fingerprint);
    //Store first bitstring
    TargetFingerPrints.push_back(aprint);
    //Store remaining bitstrings from file of fingerprints
    targetnumber_fingerprints++;
    while(cin >> fingerprint){
      if(vector_size != fingerprint.size()){
        cerr << "fingerprints of unequal size from stdin" << endl;
        exit(1);
      }
      BitVector aprint(fingerprint);
      TargetFingerPrints.push_back(aprint); 
      targetnumber_fingerprints++; 
    }
  }
  else{// Same as above, only read in from a file.
    ifs1 >> fingerprint;
    vector_size = fingerprint.size();
    BitVector aprint(fingerprint);
    TargetFingerPrints.push_back(aprint);
    targetnumber_fingerprints++;
    while(ifs1 >> fingerprint){
      if(vector_size != fingerprint.size()){
        cerr << "fingerprints of unequal size in file" << endl;
        exit(1);
      }
      BitVector aprint(fingerprint);
      TargetFingerPrints.push_back(aprint); 
      targetnumber_fingerprints++;
    }
  }

  
  if(queryfilename == "-"){
    //Read in fingerprint from standard input
    cin >> fingerprint;
    //Determine the length of the first fingerprint
    vector_size = fingerprint.size();
    //Assign the fingerprint to a bitstring (bitset)
    BitVector aprint(fingerprint);
    //Store first bitstring
    QueryFingerPrints.push_back(aprint);
    //Store remaining bitstrings from file of fingerprints
    querynumber_fingerprints++;
    while(cin >> fingerprint){
      if(vector_size != fingerprint.size()){
        cerr << "fingerprints of unequal size from stdin" << endl;
        exit(1);
      }
      BitVector aprint(fingerprint);
      QueryFingerPrints.push_back(aprint); 
      querynumber_fingerprints++; 
    }
  }
  else{// Same as above, only read in from a file.
    ifs2 >> fingerprint;
    vector_size = fingerprint.size();
    BitVector aprint(fingerprint);
    QueryFingerPrints.push_back(aprint);
    querynumber_fingerprints++;
    while(ifs2 >> fingerprint){
      if(vector_size != fingerprint.size()){
        cerr << "fingerprints of unequal size in file" << endl;
        exit(1);
      }
      BitVector aprint(fingerprint);
      QueryFingerPrints.push_back(aprint); 
      querynumber_fingerprints++;
    }
  }

  measures tanidefaultmeasure;
  for(i=0; i < targetnumber_fingerprints-1; i++){
    for(j=i+1; j < querynumber_fingerprints; j++){
      tmpmeasure = 1.0 - tanidefaultmeasure(TargetFingerPrints[i],QueryFingerPrints[j]);
      if(SparseThreshold > tmpmeasure)
        cout << j << " " << tmpmeasure << " ";
    }
    cout << -1 << endl;
  }

  return 0;
}











