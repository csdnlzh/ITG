#ifndef Feature_Manager_H
#define Feature_Manager_H
#include <unordered_map>
#include <string>
using namespace std;

class FeatureManager
{
public:
	static FeatureManager& GetSingleton()
	{
		return *instance;
	}
	long long GetFeatureCount()
	{
		return features_count;
	}
	long long GetFeatureIndex(string feature_name);
private:
	FeatureManager()
	{
		features_count=0;
	}
	static FeatureManager* instance;
	long long features_count;
    std::unordered_map<string,long long> feature_name2index_map;
};

#endif
