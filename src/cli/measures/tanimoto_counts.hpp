//Program Tanimoto
//TanimotoCounts.h 
//TanimotoCounts classes

#ifndef TANIMOTOCOUNTS_H
#define TANIMOTOCOUNTS_H

#include "Globals.h"

class tanimotocounts
{
protected:
	unsigned int length; //length of char valued vector
        char a;
        char b;

public:
	void init(unsigned int len);
	float computeMeasure(std::vector<char> vector1, std::vector<char> vector2);
};


#endif


		

