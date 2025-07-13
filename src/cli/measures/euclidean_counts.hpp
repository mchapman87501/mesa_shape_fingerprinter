//Program MeasuresCounts
//EuclideanCounts.h 
//Euclideancounts classes

#ifndef EUCLIDEANCOUNTS_H
#define EUCLIDEANCOUNTS_H

#include "Globals.h"

class euclideancounts
{
protected:
	unsigned int length; //length of char valued vector
public:
	void init(unsigned int len);
	float computeMeasure(std::vector<char> vector1, std::vector<char> vector2);
};


#endif


		

