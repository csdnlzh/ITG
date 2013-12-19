#ifndef SCFG_H
#define SCFG_H
#include <string>
#include "rule.h"
#include "symbols.h"
#include "corpus.h"
#include "phrase_table.h"
#include "globals.h"
#include "config.h"
#include "prior_knowledge.h"
#include "train_param.h"

using namespace std;

class SCFG
{

public:

	SCFG(Corpus& corpusp):corpus(corpusp)
	{
		this->start_symbol=Symbols::symbols["S"];
		this->C =Symbols::symbols["C"];
		this->nonterminal_count=Symbols::symbols.size();

		for(int i=0;i<this->nonterminal_count;i++)
		{
			this->N_cnt[i]=0;
		}

		this->max_phrase_length=ITG::Config::GetSingleton().configfile->read<int>("max_phrase_length");
		this->thresholdprob = ITG::Config::GetSingleton().configfile->read<double>("threshold");
		this->defaultprob = ITG::Config::GetSingleton().configfile->read<double>("defaultprob");
		this->debug_level = ITG::Config::GetSingleton().configfile->read<int>("debug_level");
		this->phrase_rules_count=0;
        prior_knowledge=& (Prior_Knowledge::GetInstance());
        init_em_parameters=ITG::Config::GetSingleton().configfile->read<int>("init_em_parameters");
        use_pos_feature=ITG::Config::GetSingleton().configfile->read<int>("use_pos_feature");

	}
	
    /**
     * @brief  before the multi thread training,we scan the whole corpus and create the whole 
     * one-to-one phrase rules.
     */
    void InitPhraseRules();
	void WriteTransTable(string tfile);
	void ReadPhraseGramma(string pgfile);
	void ReadGramma(string gfile);
	void WriteGramma(string gfile);
	void Update(bool vb);
	void Update(vector<double>& weight);
    
    /**
     * @brief when use multi threads for training ,we use this method for update rules prob 
     *
     * @param params the expect count of each rule trained by multi threads
     */
    void Reduce_SCFG_Params(const vector<Train_Param*>& params);
	void InitSystemRules();
	void AdjustPhraseProb();
	void ComputeTransProb();
	double ComputeThresholdProb();
	PhraseRule* Build_Phraserule(string rulestr);
	void AddSystemRules(vector<string>& rules);
	Rule* GetPhrase2PhraseRule(vector<WORDTYPE> ephrase, vector<WORDTYPE> cphrase);
    
	Rule* GetRule(int lhs,int rhsl,int rhsr,RuleType type);
	Rule* GetRule(int lhs,int rhs);
	bool IsPhraseSpan(int senpair_index,int estart,int eend,int cstart,int cend ,WORDTYPE* ephrase,WORDTYPE* cphrase);

    void Shrink(double threshold);

    //the data structure for storing the rules
    std::unordered_map<long long,Rule*> unary_rules_map;
	std::unordered_map<long long,Rule*> binary_rules_map;
	Phrase_table phrase_table;
    vector<PhraseRule*> phrase_rules;
	vector<Rule*> binary_rules;
	vector<Rule*> unary_rules;

    //the rules count;
    int unary_rules_count;
    int binary_rules_count;
	int phrase_rules_count;

	int start_symbol;        //the start symbol of the scfg
	int nonterminal_count;   
	int C ; //preterm non_terminal
	int max_phrase_length;

	double N_cnt[MAXN]; //use to store the expect count of one of the nonterminal 

	double thresholdprob;
	double defaultprob;
	int debug_level;
	Corpus& corpus;
    Prior_Knowledge* prior_knowledge;

    int init_em_parameters;
    int use_pos_feature;
    
private:
    void PhraseRule_QueryAndCreate(int sentence_id,int estart,int eend,int cstart,int cend,vector<WORDTYPE> ephrase,vector<WORDTYPE> cphrase);
};

#endif
