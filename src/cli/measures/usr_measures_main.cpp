#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>

#include "USR_Smeasure.h"

#include "mesaac_common/mesaac_common.h"

using namespace std;

string Version = "1.0";
string CreationDate = "April, 2011";

typedef vector< float > FloatVector;
typedef vector< FloatVector > VectorList;

int main(int argc, char **argv){
  
  unsigned int i,j, number_Vectors = 0;
  float SparseThreshold;
  float tmpmeasure;
  float value;
  FloatVector floatVector;
  VectorList ArrayFloatVectors;

  mesaac::MesaacFeatures f;
  f.set(mesaac::MFE_GroupingModule);
  mesaac::initCommon(f);

  cerr << "RunningUSR  Measures.  Source code Copyright (c) 2011 Mesa Analytics & Computing, Inc" << endl
       << "Version number " << Version << " Creation Date: " << CreationDate << endl
       << "Expiration Date: " << mesaac::expirationDateStr() << endl;

  if (argc != 7){
    cerr << "Usage: " << argv[0]
         << "featurevectorfile.txt measure similarity format sparsethreshold vector_size" << endl
         << "measure = '-S' for S USR measure,'-E' for Euclidean, '-C' for Cosine" << endl
         << "similarity = '-S' for similarity, '-D' for dissimilarity" << endl
         << "format = '-M' for Matrix, '-O' for Ordered Pairs, '-S' for Sparse Matrix" << endl
         << "sparsethreshold is in the range of (0,1), and format must be = '-S'" << endl
	 << "vector_size is the length of the feature vector" << endl;
    exit(1);
  }
  
  // Input from either stdin or file
  string inputstring = argv[1];
  ifstream ifs1;
  if(inputstring != "-"){
    ifs1.open(argv[1]);
    if(!ifs1){
      cerr << "Cannot open feature vector file. Abort." << endl;
      exit(1);
    }
  }                       
  
  string distanceMeasure = argv[2];
  string similarity = argv[3];
  string format = argv[4];

  if(similarity[1] != 'S' && similarity[1] != 'D'){
    cerr << "similarity option is not either S or D. Default to Dissimilarity" << endl;
  }    
  SparseThreshold = atof(argv[5]);
  unsigned int vector_size = atoi(argv[6]);

  //Read in feature vectors from feature vector file. 
  if(inputstring == "-"){
    //Read in single value from vector from standard input
    i = 0;
    while(cin >> value){
      if(i < vector_size){
	floatVector.push_back(value);
	i++;
      }
      else{
	ArrayFloatVectors.push_back(floatVector);
	floatVector.erase(floatVector.begin(),floatVector.end());
	floatVector.push_back(value);
	i = 1;
	number_Vectors++;
      }
    }
    //Insert the last remaining floatVector and erase the floatVector.
    ArrayFloatVectors.push_back(floatVector);
    floatVector.erase(floatVector.begin(),floatVector.end());
    number_Vectors++;
  }
  else{// Same as above, only read in from a file.
    i = 0;
    while(ifs1 >> value){
      if(i < vector_size){
        floatVector.push_back(value);
        i++;
      }
      else{
        ArrayFloatVectors.push_back(floatVector);
        floatVector.erase(floatVector.begin(),floatVector.end());
        floatVector.push_back(value);
	i = 1;
        number_Vectors++;
      }
    }
    //Insert the last remaining floatVector and erase the floatVector.
    ArrayFloatVectors.push_back(floatVector);
    floatVector.erase(floatVector.begin(),floatVector.end());
    number_Vectors++;
  }
  

  if(distanceMeasure[1] == 'S'){
    usr_smeasure smeasure;
    smeasure.init(vector_size);
    if(similarity[1] == 'S'){//similarity
      if(format[1] == 'O'){//ordered pair format
	for(i=0; i < number_Vectors; i++){
	  for(j=0; j < number_Vectors; j++){
	    smeasure.init(vector_size);
	    cout << i << " " << j << " " << 
	      smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]) << endl;
	  }
	}
      }
      else if(format[1] == 'M'){//Matrix format
	for(i=0; i < number_Vectors; i++){
	  for(j=0; j < number_Vectors; j++){
	    smeasure.init(vector_size);
            
	    if(j != number_Vectors - 1)
	      cout << 
		smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]) << " ";
	    else
	      cout << smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]);
	  }
	  cout << endl;
	}
      }
      else if(format[1] == 'S'){//Sparse Matrix
	for(i=0; i < number_Vectors; i++){
	  for(j=0; j < number_Vectors; j++){
	    if(i!=j){
	      smeasure.init(vector_size);             
	      tmpmeasure = 
		smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]);      
	      if(SparseThreshold < tmpmeasure)
		cout << j << " " << tmpmeasure << " ";
	    }
	  }
	  cout << -1 << endl;
	}
      }
    }
    else{// dissimilarity
      if(format[1] == 'O'){//ordered pair format
	for(i=0; i < number_Vectors; i++){
	  for(j=0; j < number_Vectors; j++){
	    smeasure.init(vector_size);
	    cout << i << " " << j << " " << 
	      1.0 - smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]) << endl;
	  }
	}
      }
      else if(format[1] == 'M'){//Matrix format
	for(i=0; i < number_Vectors; i++){
	  for(j=0; j < number_Vectors; j++){
	    smeasure.init(vector_size);
            
	    if(j != number_Vectors - 1)
	      cout << 
		1.0 - smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]) << " ";
	    //1.0 - smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]) << " ";
	    
	    else
	      cout << (1.0 - smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]));
	  }
	  cout << endl;
	}
      }
      else if(format[1] == 'S'){//Sparse Matrix
	for(i=0; i < number_Vectors; i++){
	  for(j=0; j < number_Vectors; j++){
	    if(i!=j){
	      smeasure.init(vector_size);             
	      tmpmeasure = 
		1.0 - smeasure.computeMeasure(ArrayFloatVectors[i],ArrayFloatVectors[j]);        
	      if(SparseThreshold >= tmpmeasure)
		cout << j << " " << tmpmeasure << " ";
	    }
	  }
	  cout << -1 << endl;
	}
      }
    }
  }
  else {
    cerr << "No measure chosen, check that - option set to either S, E, or C\n";
    return 1;
  }
  return 0;
}
