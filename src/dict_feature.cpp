#include <boost/algorithm/string.hpp>
#include "dict_feature.h"
#include "prior_knowledge.h"
#include "tools.h"

pair<string,double> DictFeature::GetFeatureValue(Rule& rule)
{
    PhraseRule* phrase_rule =(PhraseRule*)&rule;
    string feature_name="dict"; //+Tools::i2str(rule.rule_id);
    string cphrase=this->GetPhraseString(phrase_rule->m_cphrase,corpus->cdict_reverse);
    string ephrase=this->GetPhraseString(phrase_rule->m_ephrase,corpus->edict_reverse);
    bool in =prior_knowledge->IsContainedInDict(cphrase,ephrase);
    if(in)
    {
        return make_pair(feature_name,1);
    }
    else
    {
        return make_pair(feature_name,0);
    }
}

string DictFeature::GetPhraseString(const vector<WORDTYPE>& phrase,std::unordered_map<int,string>& dict)
{
    string phrasestring;
    for(unsigned int i=0;i<phrase.size();i++)
    {
        int index=phrase[i];
        phrasestring+= dict[index];
        phrasestring+=" ";
    }
    boost::trim(phrasestring);
    return phrasestring;
}
