#include "basic_feature.h"
#include "tools.h"
#include "rule.h"
pair<string,double> BasicFeature::GetFeatureValue(Rule& rule)
{
	PhraseRule* phrase =(PhraseRule*)&rule;
	string feature_name= "basic"+Tools::i2str(phrase->rule_id);
	return make_pair(feature_name,1.0);
}