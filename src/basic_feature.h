#ifndef Expected_Count_Feature_H
#define Expected_Count_Feature_H
#include "abstract_feature.h"

class BasicFeature : public AbstractFeature
{
public:
	BasicFeature()
	{

	}
	pair<string,double> GetFeatureValue(Rule& rule);
};

#endif