#ifndef Features_H
#define Features_H
#include <vector>
#include "rule.h"
#include "abstract_feature.h"
#include "basic_feature.h"
#include "scfg.h"
using namespace std;

class Features
{
public:
	
	static Features& GetSingleton()
	{
		return *instance;
	}
	void ComputeActiveFeatures(SCFG& scfg);
    //we use vector< pari<long long,double>> to store a rule's feature index and feature value
    //pair<long long,double> to store a feature's feature index and feature value
	vector<vector<pair<long long,double> > > rules_features_index_values;  
	void AddFeatureTemplate(AbstractFeature*);
private:
	static Features* instance;
	Features()
	{
	}
	//compute the features value of the rule
	vector<pair<long long,double> > ComputeFeatureValues(Rule& rule);
	vector<AbstractFeature* > all_feature_functions; 
	

};
#endif
