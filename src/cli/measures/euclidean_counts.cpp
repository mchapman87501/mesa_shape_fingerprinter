//Program EuclideanCounts
//Euclidean Counts class methods

#include "EuclideanCounts.h"
#include <math.h>

using namespace std;

void euclideancounts::init(unsigned int len)
{
	length = len;
}

float euclideancounts::computeMeasure(vector<char> vector1, vector<char> vector2)
{
  unsigned int i;
  float sum = 0.0;
  for(i = 0; i< length; i++){
    sum += (vector1[i] - vector2[i])*(vector1[i] - vector2[i]);
  }
  return (float) sqrt(double(sum));
}


