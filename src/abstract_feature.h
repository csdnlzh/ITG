#ifndef Abstract_Feature_H
#define Abstract_Feature_H
#include "rule.h"
#include <utility>
#include <string>
class AbstractFeature
{
public:
	AbstractFeature()
	{
		
	};
	virtual  pair<string,double> GetFeatureValue(Rule& rule)=0;
};
#endif