//Program USR S measure
//USR S measure class methods

#include "USR_Smeasure.h"
#include <cmath>

using namespace std;

void usr_smeasure::init(unsigned int len)
{
	length = len;
}

float usr_smeasure::computeMeasure(vector<float> vector1, vector<float> vector2)
{
  unsigned int i;
  float sum = 0.0;
  for(i = 0; i< length; i++){
    sum += fabs(vector1[i] - vector2[i]);
  }
  return(1.0/(1.0 + (sum/length) ));
}


