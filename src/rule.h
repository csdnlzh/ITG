#ifndef Rule_H
#define Rule_H

#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include <unordered_map>
#include "symbols.h"
#include "tools.h"
using namespace std;

enum RuleType{unary=1,bi_same,bi_reverse,term};

class Rule
{

public:
	Rule()
	{
        rule_id=0;
		this->prob=0.0;
		this->expect_count = 0.0;
		this->tran_prob = 0;
	}
	Rule(string str,int id)
	{
        rule_id = id ;
		this->expect_count=0;
		this->right1=0;
		this->right2=0;

		vector<string> prob_rulestr;

		boost::split(prob_rulestr,str,boost::algorithm::is_any_of("\t"));

		this->prob=atof(prob_rulestr[0].c_str());

		string rulestr=prob_rulestr[1];

		vector<string> lh_rh;

		boost::split(lh_rh,rulestr,boost::algorithm::is_any_of("#"));

		string leftstr=boost::trim_copy(lh_rh[0]);

		this->left=Symbols::symbols[leftstr];

		string rightstr=boost::trim_copy(lh_rh[1]);

		vector<string> rightv;

		boost::split(rightv,rightstr,boost::algorithm::is_any_of(" "));


		if(rightstr.find('[')!=string::npos)
		{
			type=bi_same;

			this->right1=Symbols::symbols[rightv[1]];

			this->right2=Symbols::symbols[rightv[2]];
		}

		if(rightstr.find('<')!=string::npos)
		{
			type=bi_reverse;

			this->right1=Symbols::symbols[rightv[1]];

			this->right2=Symbols::symbols[rightv[2]];

		}

		if(rightv.size()==1)
		{
			type=unary;
			this->right1=Symbols::symbols[rightv[0]];

		}
	}

	string Format()
	{
		string str= Tools::d2str(this->prob)+"\t"+Symbols::invert_symbols[left]+" # ";

		if(type==bi_same)
		{
			str+="[ "+Symbols::invert_symbols[right1]+" "+Symbols::invert_symbols[right2]+" ]";
		}
		if(type==bi_reverse)
		{
			str+="< "+Symbols::invert_symbols[right1]+" "+Symbols::invert_symbols[right2]+" >";

		}
		if(type==unary)
		{
			str+=Symbols::invert_symbols[right1];
		}
		return str;

	}

    int rule_id;
	double expect_count;
	double prob;
	int left;
	int right1;
	int right2;
	RuleType type;
	double tran_prob ; //translation probability of C->e|f ,used for fillter bad rules in the alignment tree, it has not been used.

 };

class PhraseRule:public Rule
{
public:
    PhraseRule(int id,double prob,vector<WORDTYPE> ephrase,vector<WORDTYPE> cphrase);
	vector<WORDTYPE> m_ephrase;
	vector<WORDTYPE> m_cphrase;

    //used for tag feature , a rule can have several c words , a word can have several tags, we use a map to store the tag id and the 
    //occurs times ,the same to e words.
    vector<std::unordered_map<int,int> > cwords_tags;
    vector<std::unordered_map<int,int> > ewords_tags;
    void AddTags(const vector<int>& c_tags,const vector<int>& e_tags);
    void ClearTags();
    string Format();
    string StrFormat();
};

#endif

