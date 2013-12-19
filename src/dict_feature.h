#ifndef Dict_feature_H
#define Dict_feature_H
#include <string>
#include <unordered_map>
#include <vector>
#include "abstract_feature.h"
#include "corpus.h"
#include "prior_knowledge.h"

using namespace std;

class DictFeature:public AbstractFeature
{
public:
    DictFeature()
    {
        corpus= &Corpus::GetSingleton();
        prior_knowledge= &Prior_Knowledge::GetInstance();
    }
    pair<string,double> GetFeatureValue(Rule& rule);
    string GetPhraseString(const vector<WORDTYPE>& phrase,unordered_map<int,string>& dict);
private:
    Corpus* corpus;
    Prior_Knowledge* prior_knowledge;
};
#endif
