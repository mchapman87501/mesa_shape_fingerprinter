//Program MeasuresCounts
//USR_Smeasure.h
//USR S measure classes

#ifndef USR_SMEASURE_H
#define USR_SMEASURE_H

#include <vector>

class usr_smeasure
{
protected:
	unsigned int length; //length of float valued vector
public:
	void init(unsigned int len);
	float computeMeasure(std::vector<float> vector1, std::vector<float> vector2);
};


#endif


		

