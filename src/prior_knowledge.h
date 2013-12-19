#ifndef Prior_Knowledge_H
#define Prior_Knowledge_H
#include <vector>
#include <unordered_map>
#include "globals.h"
#include "rule.h"
#include "corpus.h"

using namespace std;

class Prior_Knowledge
{

public:
	vector<vector<vector<int> > >& Model4_Intersection();
    static Prior_Knowledge& GetInstance()
    {
        if(!singleton)
        {
            singleton = new Prior_Knowledge();
        }
        return *singleton;
    }
    void ReadLinks();
    void ReadInitProb();
    void ReadTagProb();
    void ReadTagVocabulary();
    void ReadSentenceTags();
    void ReadSentenceTagsFile(const string& filename,vector<vector<int> >& sentences_tags,std::unordered_map<string,int>& dict);
    void FillTranTable(const string& fileName,Tran_table& table);
    double GetRuleInitProb(const Rule& rule);
    double GetRuleTagsProb(const Rule& rule);

    //we try to use universal tags for compute score
    double GetRuleTagsScore(const Rule& rule);
    void GetCPhraseTags(int sentence_id,int start,int end,vector<int>& words_tags);
    void GetEPhraseTags(int sentence_id,int start,int end,vector<int>& words_tags);
    void BuildTagVocabulary(const string& filename,std::unordered_map<string,int>& dict, std::unordered_map<int,string>& dict_reverse);
    void DisplayRuleTags(const Rule& rule);

    void ReadDictfile();
    bool IsContainedInDict(const string& cphrase,const string& ephrase);
    
    void ReadPunctuation();
    bool IsPunctuation(string term);
private:
	Prior_Knowledge();
    static Prior_Knowledge* singleton;
    Tran_table c2e_table_forInit;
    Tran_table e2c_table_forInit;
	vector<vector<vector<int> > > intersection_links;
    double GetItem2ItemProb(int item1,int item2,const Tran_table& trans_table);
    double GetPhrase2PhraseProb(const vector<WORDTYPE>& phrase1,const vector<WORDTYPE>& phrase2,const Tran_table& tran_table);

    //for pos feature
    Tran_table c2e_tagtable;
    Tran_table e2c_tagtable;
    double GetTags2TagsProb_forWords(const vector<std::unordered_map<int,int> >& src_tags,const vector<std::unordered_map<int,int> >& trg_tags,const Tran_table& tag_table);
    double GetTags2TagsProb_forWord(const std::unordered_map<int,int>& src_tags,const std::unordered_map<int,int>& trg_tags,const Tran_table& tag_table);
    vector<vector<int> > c_sentences_tags;
    vector<vector<int> > e_sentences_tags;
    std::unordered_map<string,int> cpos_map;
    std::unordered_map<string,int> epos_map; 
    std::unordered_map<int,string> cpos_map_reverse;
    std::unordered_map<int,string> epos_map_reverse;
    
    //we try to use universal tags
    std::unordered_map<string,int> pos_map;
    std::unordered_map<int,string> pos_map_reverse;

    //a parallel language dict
    Dict_table dict;
    Corpus* corpus;

    std::unordered_map<string,int> punctuation_map;

    int debug_level;
};
#endif
