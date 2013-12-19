#include "features_manager.h"
FeatureManager* FeatureManager::instance = new FeatureManager();

long long FeatureManager::GetFeatureIndex(string feature_name)
{
	long long index;
	unordered_map<string,long long>::iterator index_iterator =this->feature_name2index_map.find(feature_name);
	if(index_iterator!=this->feature_name2index_map.end())
	{
		index=index_iterator->second;
	}
	else
	{
		feature_name2index_map[feature_name]=this->features_count;
		index=features_count;
		features_count++;
	}
	return index;
}
