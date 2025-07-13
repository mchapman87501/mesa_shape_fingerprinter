//Program MeasuresCounts
//CosineCounts.h 
//Cosinecounts classes

#ifndef COSINECOUNTS_H
#define COSINECOUNTS_H

#include "Globals.h"

class cosinecounts
{
protected:
	unsigned int length; //length of char valued vector
public:
	void init(unsigned int len);
	float computeMeasure(std::vector<char> vector1, std::vector<char> vector2);
};


#endif


		

