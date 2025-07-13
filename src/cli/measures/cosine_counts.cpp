//Program MeasuresCounts
//Cosine Counts class methods

#include "CosineCounts.h"
#include <math.h>
#include <vector>

using namespace std;

void cosinecounts::init(unsigned int len)
{
	length = len;
}

float cosinecounts::computeMeasure(vector<char> vector1, vector<char> vector2)
{
  unsigned int i;
  float dot = 0.0;
  float suma = 0.0;
  float sumb = 0.0;
  for(i = 0; i< length; i++){
      dot += vector1[i] * vector2[i];
      suma += vector1[i] * vector1[i];
      sumb += vector2[i] * vector2[i];
  }
  return (float) (dot/(sqrt(double(suma))*sqrt(double(sumb))));
}


