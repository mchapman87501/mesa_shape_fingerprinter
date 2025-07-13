//Program MeasuresCounts
//TanimotoCounts class methods

#include "TanimotoCounts.h"
#include <iostream>

using namespace std;

void tanimotocounts::init(unsigned int len)
{
        length = len;
        a = 0;
        b = 0;
}

float tanimotocounts::computeMeasure(vector<char> vector1, vector<char> vector2)
{

    unsigned int i;
    float sum = 0.0;
    for(i = 0; i< length; i++){
        a = vector1[i];
        b = vector2[i];
        //cerr << (float) a << " " << (float) b << endl;
        if(a == 0 || b == 0) sum += 0;
        else if(a < b) sum += (float)a/(float)b;
        else sum += (float)b/(float)a;
    }
    return sum;       
}

