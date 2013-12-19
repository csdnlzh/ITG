#ifndef Aligner_H
#define Aligner_H
#include <fstream>
#include <ctime>
#include <sstream>
#include "senpair.h"
#include "table_cell.h"
#include "corpus.h"
#include "scfg.h"
using namespace std;


//Compute the best alignment according the ITG Parsing Tree.
//By default, We just get the alignment from the itg unary rules.
//We can incorporate other information in order to get better alignment.
class Aligner
{
public:

    Aligner(int id,Corpus& corpus, SCFG& scfg, vector<SenPair*>& pairs,int start,int end,vector<string>& alignments):m_scfg(scfg),m_senpairs(pairs),m_alignments(alignments),m_corpus(corpus)
                                                                                                                  
    {
        this->m_aligner_id =id;
        this->m_sen_start=start;
        this->m_sen_end=end;
        this->m_rule_prob_threshold=0;
    }

    void SetRuleProbThreshold(double threshold)
    {
        this->m_rule_prob_threshold=threshold;
    }

    void operator()();

    ~Aligner()
	{

	}

    int multi_words_rules_count;
private:
    
    //used for multi thread runing
    int m_aligner_id;

    int m_sen_start;
    int m_sen_end;
    SCFG& m_scfg;
    vector<SenPair*>& m_senpairs;
    vector<string>& m_alignments;
    Corpus& m_corpus;
	//release the chart parsing table that the itg parser create.
	void ReleaseTable();

	//get the unary itg rules from top to down recursivly.
	void CollectRules(ParseTableCell* cell,int lhs);

	//get the word position in the sentence
	int GetPosition(string lang,int word);

	//write the alignment result to files.
	void WriteAlignment();

	vector<Rule*> best_tree_rules;
	ParseTableCell * ptable;
	SenPair* senpair;
	int elen;
	int clen;
    int e_dim;
	int c_dim;
	int e_dim_c_dim_c_dim;
	int c_dim_c_dim;

	double m_rule_prob_threshold;
	inline int GetArrayIndex(int s,int t,int u,int v)
	{
		return s*(e_dim_c_dim_c_dim)+t*(c_dim_c_dim)+u*c_dim+v;
	}
};
#endif
