#include "align_features.h"
#include "features_manager.h"

Features* Features::instance = new Features();


/**
 * @brief Compute feature values for a rule.
 *
 * @param rule
 *
 * @return 
 */
vector<pair<long long,double> > Features::ComputeFeatureValues(Rule& rule)
{
	vector<pair<long long,double> >  feature_id_values;
	for(unsigned int i=0;i<this->all_feature_functions.size();i++)
	{
		pair<string,double> feature_name_value= this->all_feature_functions[i]->GetFeatureValue(rule);
		long long feature_index = FeatureManager::GetSingleton().GetFeatureIndex(feature_name_value.first);
		feature_id_values.push_back(make_pair(feature_index,feature_name_value.second));
	}
	return feature_id_values;
}


/**
 * @brief Compute feature values for the grammar.
 *
 * @param scfg
 */
void Features::ComputeActiveFeatures(SCFG& scfg)
{
	this->rules_features_index_values.clear();
	this->rules_features_index_values.resize(scfg.phrase_rules_count);
    for(int i=0;i<scfg.phrase_rules_count;i++)
    {
        PhraseRule* rule = scfg.phrase_rules[i];
        this->rules_features_index_values[rule->rule_id]=this->ComputeFeatureValues(*rule);
    }
}


/**
 * @brief Add a feature function to the model.
 *
 * @param feature The added feature, e.g. basic feature, pos feature, dictionary feature.
 */
void Features::AddFeatureTemplate(AbstractFeature* feature)
{
	this->all_feature_functions.push_back(feature);
}
