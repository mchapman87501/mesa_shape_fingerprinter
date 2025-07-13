#include <fstream>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <cstdlib>

#include "Globals.h"
#include "EuclideanCounts.h"
#include "TanimotoCounts.h"
#include "TverskyCounts.h"
#include "CosineCounts.h"

#include "mesaac_common/mesaac_common.h"

using namespace std;

string Version = "1.0";
string CreationDate = "May, 2007";


int main(int argc, char **argv){
  
  unsigned int i,j, number_countVectors = 0;
  float SparseThreshold;
  float tmpmeasure;
  float TverskyAlpha;
  unsigned int count;
  char charcount;
  vector<char> countsVector;

  
  mesaac::MesaacFeatures f;
  f.set(mesaac::MFE_GroupingModule);
  mesaac::initCommon(f);

  cerr << "Running Measures.  Source code Copyright (c) 2007 Mesa Analytics & Computing, LLC" << endl
       << "Version number " << Version << " Creation Date: " << CreationDate << endl
       << "Expiration Date: " << mesaac::expirationDateStr() << endl;

  if (argc != 7){
      cerr << "Usage: " << argv[0]
         << "countsfile.txt measure similarity format sparsethreshold alpha" << endl
         << "measure = '-T' for Tanimoto,'-V' for Tversky, '-E' for Euclidean, '-C' for Cosine" << endl
         << "similarity = '-S' for similarity, '-D' for dissimilarity" << endl
         << "format = '-M' for Matrix, '-O' for Ordered Pairs, '-S' for Sparse Matrix" << endl
         << "sparsethreshold is in the range of (0,1), and format must be = '-S'" << endl;
    exit(1);
  }
  
  // Input from either stdin or file
  string inputstring = argv[1];
  ifstream ifs1;
  if(inputstring != "-"){
    ifs1.open(argv[1]);
    if(!ifs1){
      cerr << "Cannot open counts file. Abort." << endl;
      exit(1);
    }
  }                       
  
  string distanceMeasure = argv[2];
  string similarity = argv[3];
  string format = argv[4];
  
  if(similarity[1] != 'S' && similarity[1] != 'D'){
    cerr << "similarity option is not either S or D. Default to Dissimilarity" << endl;
  }    

  //Read in fingerprints from fingerprint file. 
  SparseThreshold = atof(argv[5]);
  TverskyAlpha = atof(argv[6]);
  
  //Create a list of fingerprints to store each bitstring in
  ArrayCountVectors CountVectors;
  //Length of count vector
  unsigned int vector_size = FPLEN;


  if(inputstring == "-"){
      //Read in single counts from count fingerprints from standard input
      i = 0;
      while(cin >> count){
          if(i < vector_size){
              charcount = count;
              countsVector.push_back(charcount);
              i++;
          }
          else{
              CountVectors.push_back(countsVector);
              countsVector.erase(countsVector.begin(),countsVector.end());
              charcount = count;
              countsVector.push_back(charcount);
              i = 1;
              number_countVectors++;
          }
      }
      //Insert the last remaining countsVector and erase the countsVector.
      CountVectors.push_back(countsVector);
      countsVector.erase(countsVector.begin(),countsVector.end());
      number_countVectors++;
  }
  else{// Same as above, only read in from a file.
     //Read in single counts from count fingerprints from standard input
      i = 0;
      while(ifs1 >> count){
          if(i < vector_size){
              charcount = count;
              countsVector.push_back(charcount);
              i++;
          }
          else{
              CountVectors.push_back(countsVector);
              countsVector.erase(countsVector.begin(),countsVector.end());
              charcount = count;
              countsVector.push_back(charcount);
              i = 1;
              number_countVectors++;
          }
      }
      //Insert the last remaining countsVector and erase the countsVector.
      CountVectors.push_back(countsVector);
      countsVector.erase(countsVector.begin(),countsVector.end());
      number_countVectors++;
  }


  //Measures
  if(distanceMeasure[1] == 'T'){
    tanimotocounts tanicountmeasure;
    tanicountmeasure.init(vector_size);
    float max = 0.0;
    float tmpmax = 0.0;
    for(i=0; i < number_countVectors; i++){
        tanicountmeasure.init(vector_size);
        tmpmax = tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[i]);
        if(tmpmax > max){
            max = tmpmax;
        }
    }
    if(max <= 0){
      cerr << "Cannot normalize Tanimoto:  Max self Tanimoto <= 0" << endl;
      exit(1);
    }

    if(similarity[1] == 'S'){//similarity
      if(format[1] == 'O'){//ordered pair format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            tanicountmeasure.init(vector_size);
            cout << i << " " << j << " " << 
                tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max << endl;
          }
        }
      }
      else if(format[1] == 'M'){//Matrix format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            tanicountmeasure.init(vector_size);
            
            if(j != number_countVectors - 1)
              cout << 
                 tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max << " ";
            else
              cout << tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max;
          }
          cout << endl;
        }
      }
      else if(format[1] == 'S'){//Sparse Matrix
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            if(i!=j){
              tanicountmeasure.init(vector_size);             
              tmpmeasure = 
                  tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max;      
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
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            tanicountmeasure.init(vector_size);
            cout << i << " " << j << " " << 
                1.0 - tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max << endl;
          }
        }
      }
      else if(format[1] == 'M'){//Matrix format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            tanicountmeasure.init(vector_size);
            
            if(j != number_countVectors - 1)
              cout << 
                  (max - tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j]))/max << " ";
                  //1.0 - tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max << " ";

            else
                cout << (max - tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j]))/max;
          }
          cout << endl;
        }
      }
      else if(format[1] == 'S'){//Sparse Matrix
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            if(i!=j){
              tanicountmeasure.init(vector_size);             
              tmpmeasure = 
                  1.0 - tanicountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max;        
              if(SparseThreshold >= tmpmeasure)
                cout << j << " " << tmpmeasure << " ";
              }
          }
          cout << -1 << endl;
        }
      }
    }
  }
  else if(distanceMeasure[1] == 'V'){


    if(TverskyAlpha > 2.0 || TverskyAlpha < 0.0)
      cout << "Alpha is not in the range 0 to 2 -- hope that is ok. Normalization is based on this range." << endl;

    tverskycounts tvercountmeasure;
    float max = 0.0;
    float tmpmax = 0.0;
    for(i=0; i < number_countVectors; i++){
        tvercountmeasure.init(vector_size,TverskyAlpha);        
        tmpmax = tvercountmeasure.computeMeasure(CountVectors[i],CountVectors[i]);
        if(tmpmax > max){
            max = tmpmax;
        }
        tvercountmeasure.init(vector_size,2.0 - TverskyAlpha);  
        tmpmax = tvercountmeasure.computeMeasure(CountVectors[i],CountVectors[i]);
        if(tmpmax > max){
            max = tmpmax;
        }
    }
    if(max <= 0){
      cerr << "Cannot normalize Tversky:  Max self Tversky (calculated with both alpha and beta) <= 0" << endl;
      exit(1);
    }

    if(similarity[1] == 'S'){//similarity
      if(format[1] == 'O'){//ordered pair format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            //asymmetry is stored in the triangular portions of the matrices.
            //upper
            
            if(i<j){
              tvercountmeasure.init(vector_size,TverskyAlpha);  
              cout << i << " " << j << " " << 
                  tvercountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max << endl;
            }
            else if(j<i){
              tvercountmeasure.init(vector_size,2.0 - TverskyAlpha);    
              cout << i << " " << j << " " << 
                  tvercountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max << endl;
            }
            else{
              cout << i << " " << j << " " << 1.0 << endl;
            }
          }
        }
      }
      else if(format[1] == 'M'){//Matrix format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            //asymmetry is stored in the triangular portions of the matrices.
            //upper
            
            if(i<j){
              tvercountmeasure.init(vector_size,TverskyAlpha);  
              if(j != number_countVectors - 1){
                cout << tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max << " ";
              }
              else{
                cout << tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max;
              }
            }
            else if(j<i){
              tvercountmeasure.init(vector_size,2.0 - TverskyAlpha);    
              if(j != number_countVectors - 1){
                cout << tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max << " ";
              }
              else{
                cout << tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max;
              }
            }
            else{
              if(j != number_countVectors - 1){
                cout << 1.0 << " ";
              }
              else{
                cout << 1.0;
              }
            }
          }
          cout << endl;
        }
      }
      else if(format[1] == 'S'){//Sparse Matrix
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            if(i!=j){
              tvercountmeasure.init(vector_size,TverskyAlpha);  
              tmpmeasure = tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max;
              if(SparseThreshold < tmpmeasure)
                cout << j << " " << tmpmeasure << " ";
            }
          }
          cout << -1 << endl;
        }
      }
    }
    else{//dissimilarity
      if(format[1] == 'O'){//ordered pair format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            //asymmetry is stored in the triangular portions of the matrices.
            //upper
            
            if(i<j){
              tvercountmeasure.init(vector_size,TverskyAlpha);  
              cout << i << " " << j << " " << 
                  1.0 - tvercountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max << endl;
            }
            else if(j<i){
              tvercountmeasure.init(vector_size,2.0 - TverskyAlpha);    
              cout << i << " " << j << " " << 
                  1.0 - tvercountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/max << endl;
            }
            else{
              cout << i << " " << j << " " << 0.0 << endl;
            }
          }
        }
      }
      else if(format[1] == 'M'){//Matrix format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            //asymmetry is stored in the triangular portions of the matrices.
            //upper
            
            if(i<j){
              tvercountmeasure.init(vector_size,TverskyAlpha);  
              if(j != number_countVectors - 1){
                cout << 1.0 - tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max << " ";
              }
              else{
                cout << 1.0 - tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max;
              }
            }
            else if(j<i){
              tvercountmeasure.init(vector_size,2.0 - TverskyAlpha);    
              if(j != number_countVectors - 1){
                cout << 1.0 - tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max << " ";
              }
              else{
                cout << 1.0 - tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max;
              }
            }
            else{
              if(j != number_countVectors - 1){
                cout << 0.0 << " ";
              }
              else{
                cout << 0.0;
              }
            }
          }
          cout << endl;
        }
      }
      else if(format[1] == 'S'){//Sparse Matrix
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            if(i!=j){
              tvercountmeasure.init(vector_size,TverskyAlpha);  
              tmpmeasure = 1.0 - tvercountmeasure.computeMeasure(CountVectors[j],CountVectors[i])/max;
              if(SparseThreshold >= tmpmeasure)
                cout << j << " " << tmpmeasure << " ";
            }
          }
          cout << -1 << endl;
        }
      }
    }
  }//End else if
  else if(distanceMeasure[1] == 'E'){
    euclideancounts euclidcountmeasure;
    int max = 0;
    int tmpmax = 0;
    for(i=0; i < number_countVectors; i++){
        for(j=0; j < vector_size; j++){
            tmpmax += CountVectors[i][j];
        }
        if(max <= tmpmax){
            max = tmpmax;
        }
    }
    double doublemax = sqrt((double)max);
    if(max <= 0){
      cerr << "Cannot normalize Euclidean:  Max counts for any vector is 0" << endl;
      exit(1);
    }
    if(similarity[1] == 'S'){
      if(format[1] == 'O'){//ordered pair format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            euclidcountmeasure.init(vector_size);
            cout << i << " " << j << " " << 
                1.0 - euclidcountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/doublemax << endl;
          }
        }
      }
      else if(format[1] == 'M'){//Matrix format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            euclidcountmeasure.init(vector_size);
            if(j != number_countVectors - 1)
              cout << 1.0 - euclidcountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/doublemax << " ";
            else
              cout << 1.0 - euclidcountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/doublemax;
          }
          cout << endl;
        }
      }
      else if(format[1] == 'S'){//Sparse Matrix
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            if(i!=j){
              euclidcountmeasure.init(vector_size);
              tmpmeasure = 1.0 - euclidcountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/doublemax;
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
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            euclidcountmeasure.init(vector_size);
            cout << i << " " << j << " " << 
                euclidcountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/doublemax << endl;
          }
        }
      }
      else if(format[1] == 'M'){//Matrix format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            euclidcountmeasure.init(vector_size);
            if(j != number_countVectors - 1)
              cout << euclidcountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/doublemax << " ";
            else
              cout << euclidcountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/doublemax;
          }
          cout << endl;
        }
      }
      else if(format[1] == 'S'){//Sparse Matrix
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            if(i!=j){
              euclidcountmeasure.init(vector_size);
              tmpmeasure = euclidcountmeasure.computeMeasure(CountVectors[i],CountVectors[j])/doublemax;
                if(SparseThreshold >= tmpmeasure)
                  cout << j << " " << tmpmeasure << " ";
            }
          }
          cout << -1 << endl;
        }
      }
    }
  }//End else if
  else if(distanceMeasure[1] == 'C'){
    cosinecounts cosinecountmeasure;
    if(similarity[1] == 'S'){
      if(format[1] == 'O'){//ordered pair format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            cosinecountmeasure.init(vector_size);
            cout << i << " " << j << " " << 
                cosinecountmeasure.computeMeasure(CountVectors[i],CountVectors[j]) << endl;
          }
        }
      }
      else if(format[1] == 'M'){//Matrix format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            cosinecountmeasure.init(vector_size);
            if(j != number_countVectors - 1)
              cout << cosinecountmeasure.computeMeasure(CountVectors[i],CountVectors[j]) << " ";
            else
              cout << cosinecountmeasure.computeMeasure(CountVectors[i],CountVectors[j]);
          }
          cout << endl;
        }
      }
      else if(format[1] == 'S'){//Sparse Matrix
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            if(i!=j){
              cosinecountmeasure.init(vector_size);
              tmpmeasure = cosinecountmeasure.computeMeasure(CountVectors[i],CountVectors[j]);
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
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            cosinecountmeasure.init(vector_size);
            cout << i << " " << j << " " << 
                1.0 - cosinecountmeasure.computeMeasure(CountVectors[i],CountVectors[j]) << endl;
          }
        }
      }
      else if(format[1] == 'M'){//Matrix format
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            cosinecountmeasure.init(vector_size);
            if(j != number_countVectors - 1)
              cout << 1.0 - cosinecountmeasure.computeMeasure(CountVectors[i],CountVectors[j]) << " ";
            else
              cout << 1.0 - cosinecountmeasure.computeMeasure(CountVectors[i],CountVectors[j]);
          }
          cout << endl;
        }
      }
      else if(format[1] == 'S'){//Sparse Matrix
        for(i=0; i < number_countVectors; i++){
          for(j=0; j < number_countVectors; j++){
            if(i!=j){
              cosinecountmeasure.init(vector_size);
              tmpmeasure = 1.0 - cosinecountmeasure.computeMeasure(CountVectors[i],CountVectors[j]);
              if(SparseThreshold >= tmpmeasure)
                cout << j << " " << tmpmeasure << " ";
              }
          }
          cout << -1 << endl;
        }
      }
    }
  }//End else if
  else {
    cerr << "No measure chosen, check that - option set to either T, V, E, or C\n";
    return 1;
  }
  return 0;
}











