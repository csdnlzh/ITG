#include "rule.h"
#include "corpus.h"
#include "tools.h"
#include <iostream>
using namespace std;

void PhraseRule::AddTags(const vector<int> & cwords_tags_p,const vector<int> & ewords_tags_p)
{
    //for each cword
    for(unsigned int j=0;j<cwords_tags_p.size();j++)
    { 

        int tag_id=cwords_tags_p[j];
        unordered_map<int,int>::iterator it=this->cwords_tags[j].find(tag_id);
        if(it!=this->cwords_tags[j].end())
        {
            it->second++;
        }
        else
        {
            this->cwords_tags[j][tag_id]=1;
        }

    }

    //for each eword
    for(unsigned int j=0;j<ewords_tags_p.size();j++)
    {
       
        int tag_id=ewords_tags_p[j];
        unordered_map<int,int>::iterator it =this->ewords_tags[j].find(tag_id);
        if(it!=this->ewords_tags[j].end())
        {
            it->second++;
        }
        else
        {
            this->ewords_tags[j][tag_id]=1;
        }

    }
}
void PhraseRule::ClearTags()
{
    for(unsigned int i=0;i<this->cwords_tags.size();i++)
    {
        cwords_tags[i].clear();
    }
    for(unsigned int i=0;i<this->ewords_tags.size();i++)
    {
        ewords_tags[i].clear();
    }
}
PhraseRule::PhraseRule(int id,double prob,vector<WORDTYPE> ephrase,vector<WORDTYPE> cphrase)
{
    this->rule_id = id;
    this->prob=prob;
    this->left=3 ; //Symbols::symbols["C"];
    this->type=term;
    this->m_ephrase=ephrase;
    this->m_cphrase=cphrase;

    for(unsigned int i=0;i<cphrase.size();i++)
    {
        std::unordered_map<int,int> tags_map;
        cwords_tags.push_back(tags_map);
    }
    for(unsigned int i=0;i<ephrase.size();i++)
    {
        std::unordered_map<int,int> tags_map;
        ewords_tags.push_back(tags_map);
    }
}

// for machine reading
string PhraseRule::Format()
{
    string cstring ,estring;
    
    for(unsigned int i=0;i<m_ephrase.size();i++)
    {
        int eindex=m_ephrase[i];
        estring+=Tools::i2str(eindex)+" ";
    }
    for(unsigned int i=0;i<m_cphrase.size();i++)
    {
        int cindex=m_cphrase[i];
        cstring+=Tools::i2str(cindex)+" ";
    }
    return Tools::d2str(this->prob)+"\t C -> "+estring+" | "+cstring;
}

// for human reading 
string PhraseRule::StrFormat()
{
    Corpus& corpus = Corpus::GetSingleton();
    string cstring;
    string estring;
    for(unsigned int i=0;i<m_ephrase.size();i++)
    {
        int eindex=m_ephrase[i];
        estring+=corpus.edict_reverse[eindex]+" ";
    }

    for(unsigned int i=0;i<m_cphrase.size();i++)
    {
        int cindex=m_cphrase[i];
        cstring+=corpus.cdict_reverse[cindex]+" ";
    }

    return Tools::d2str(this->tran_prob)+"\t"+estring+" | "+cstring;
}
