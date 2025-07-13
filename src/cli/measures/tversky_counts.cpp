//Program MeasureCounts
//TverskyCounts subclass methods.

#include "TverskyCounts.h"

using namespace std;

void tverskycounts::init(unsigned int len, float analpha)
{
        length = len;
        a = 0;
        b = 0;
        alpha = analpha;
        beta = 2.0 - alpha;

}

float tverskycounts::computeMeasure(vector<char> vector1, vector<char> vector2)
{
    unsigned int i;
    float sum = 0.0;
    for(i = 0; i< length; i++){
        a = vector1[i];
        b = vector2[i];
        if(a == 0 || b == 0) sum += 0;
        else if(a < b) sum += (float)a/(beta*(float)b);
        else sum += (float)b/(alpha*(float)a);
    }
    return sum;
}

