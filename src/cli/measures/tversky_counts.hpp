//Program MeasuresCount
//TverskyCounts.h
//Tversky measure sub-class

#ifndef TVERSKYCOUNTS_H
#define TVERSKYCOUNTS_H

#include "Globals.h"
#include "TanimotoCounts.h"

class tverskycounts : public tanimotocounts
{
protected:
  float alpha;
  float beta;
public:
        void init(unsigned int len, float a);
        float computeMeasure(std::vector<char> vector1, std::vector<char> vector2);
};

#endif
