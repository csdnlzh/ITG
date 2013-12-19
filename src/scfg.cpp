#include <fstream>
#include <iomanip>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <math.h>
#include "scfg.h"
#include "rule.h"
#include "tools.h"
#include "symbols.h"
#include "phrase_table.h"
#include "loger.h"
#include "prior_knowledge.h"
#include "align_features.h"
using namespace std;


/**
 * @file scfg.cpp
 * @brief  the implement file of ITG grammar.
 * @author lizhonghua
 * @version 0.1
 * @date 2012-03-27
 */



/**
 * @brief Build a PhraseRule.
 *
 * @param rulestr the itg rule in pure string format
 *
 * @return a pointer to the PhraseRule
 */
PhraseRule* SCFG::Build_Phraserule(string rulestr)
{
	stringstream rulestream(rulestr);
	double prob;
	rulestream>>prob;
	char lhs;
	rulestream>>lhs;
	char con;
	rulestream>>con;
	rulestream>>con;
    
    vector<int> rhsl_v;
	vector<int> rhsr_v;

    string str;
    bool left=true;
    while(rulestream>>str)
    {
        if(str!="|")
        { 
            if(left)
            {
                rhsl_v.push_back(atoi(str.c_str()));
            }
            else
            {
                rhsr_v.push_back(atoi(str.c_str()));
            }
        }
        else
        {
            left=false;
        }
    }
	PhraseRule* rule=new PhraseRule(0,prob,rhsl_v,rhsr_v);
	return rule;
}



/**
 * @brief read the extracted phrase grammar in the second training stage. This function also removes the phrase rule which occurs not frequently.  
 *
 * @param fgfile the grammar file name.
 */
void SCFG::ReadPhraseGramma(string fgfile)
{
	cout<<"start read extract phrase grammar ..."<<fgfile<<endl;
	ifstream infile(Tools::GetFileFullName(fgfile));
    ofstream outfile(Tools::GetFileFullName("filted_phrase.txt"));
	if(!infile)
	{
		cout<<"read phrase grammar file error! "<<endl;
	}

	vector<PhraseRule* > phrase_rules;

    std::unordered_map<string,int> rule_map;
    
	while(!infile.eof())
	{
		string rulestr;
		getline(infile,rulestr);
		if(rulestr.length()==0 ) continue;
        if(rule_map.find(rulestr)!=rule_map.end())
        {
            rule_map[rulestr]=rule_map[rulestr]+1;
        }
        else
        {
            rule_map[rulestr]=1;
        }
	}
    cout<<"unfilter count: "<<rule_map.size()<<endl;
    std::unordered_map<string,int>::iterator it=rule_map.begin();
    int threshcount=ITG::Config::GetSingleton().configfile->read<int>("threshcount");
    for(;it!=rule_map.end();it++)
    {
        if(it->second>= threshcount)
        {
            PhraseRule* rule =this->Build_Phraserule(it->first);
            phrase_rules.push_back(rule);
            outfile<<rule->StrFormat()<<endl;
        }
    }
    cout<<"after filter count: "<<phrase_rules.size()<<endl;
	double prob_normalize = ITG::Config::GetSingleton().configfile->read<double>("prob_normalize");
	prob_normalize=1-prob_normalize;
	double phrase_prob = prob_normalize/double(phrase_rules.size());
	for(unsigned int i=0;i<phrase_rules.size();i++)
	{
		PhraseRule* rule = phrase_rules[i];
		rule->rule_id = this->phrase_rules_count;
		rule->prob = phrase_prob;
		phrase_table[rule->m_ephrase][rule->m_cphrase]=rule;
        this->phrase_rules.push_back(rule);
		this->phrase_rules_count++;
	}
	infile.close();
    outfile.close();
	cout<<"read phrase grammar ok. "<<endl;
}



/**
 * @brief read all the itg grammar from a file
 *
 * @param gfile grammar file name
 */
void SCFG::ReadGramma(string gfile)
{
    gfile= Tools::GetFileFullName(gfile);
	ifstream infile(gfile);
   	cout<<"start read all grammar rules..."<<gfile<<endl;
	vector<string> sys_rules;
	while(!infile.eof())
	{
		string rulestr;
		getline(infile,rulestr);
		if(rulestr.length()==0) continue ;
		stringstream rulestream(rulestr);

		double prob;
		rulestream>>prob;
		char lhs;
		rulestream>>lhs;

		if(lhs=='C')
		{
			PhraseRule* rule=this->Build_Phraserule(rulestr);
			rule->rule_id = this->phrase_rules_count;
			phrase_table[rule->m_ephrase][rule->m_cphrase]=rule;
            this->phrase_rules.push_back(rule);
			this->phrase_rules_count++;
		}
		else
		{
			sys_rules.push_back(rulestr);
		}
	}
	cout<<"read all grammar rules ok. "<<endl;
	this->AddSystemRules(sys_rules);
	infile.close();
}


/**
 * @brief write human readable grammar.
 *
 * @param tfile
 * @param corpus
 */
void SCFG::WriteTransTable(string tfile)
{
    this->ComputeTransProb();
	ofstream human_outfile(Tools::GetFileFullName("human_"+tfile),ios::out);
    for(int i=0;i<phrase_rules_count;i++)
    {
        if(this->phrase_rules[i]->tran_prob>0.8)
             human_outfile<<this->phrase_rules[i]->StrFormat()<<endl;
    }
	//outfile.close();
	human_outfile.close();
}


/**
 * @brief write machine readable grammar
 *
 * @param gfile
 */
void SCFG::WriteGramma(string gfile)
{
    string filename=Tools::GetFileFullName(gfile);
    cout<<"in scfg WriteGramma"<<filename<<endl;
	ofstream outfile(filename.c_str(),ios::out);
    if(!outfile)
    {
        cout<<"open file failed!"<<endl;
        return ;
    }
	for(int i=0;i<this->unary_rules_count;i++)
	{
		outfile<<this->unary_rules[i]->Format();
		outfile<<endl;
	}
	for(int i=0;i<this->binary_rules_count;i++)
	{
		outfile<<this->binary_rules[i]->Format();
		outfile<<endl;
	}

    for(int i=0;i<this->phrase_rules_count;i++)
    {
        outfile<<this->phrase_rules[i]->Format()<<endl;
    }
	outfile.close();

}



/**
 * @brief After the E_step, we need to re_estimate the rule's probabilities. 
 * This method is the M_step of EM algorithm. 
 * It's for the traditional ITG.
 *
 * @param vb  If use the variational bayesian method.
 */
void SCFG::Update(bool vb)
{
	//update the unary rules
	for(int i=0;i<this->unary_rules_count;i++)
	{
		Rule* rule=this->unary_rules[i];
		double new_prob=rule->expect_count/this->N_cnt[rule->left];
		rule->prob= new_prob<this->thresholdprob ? this->thresholdprob : new_prob;
		rule->expect_count=0;
	}
	//update the bianry rules
	for(int i=0;i<this->binary_rules_count;i++)
	{
		Rule* rule=this->binary_rules[i];
		double new_prob=rule->expect_count/this->N_cnt[rule->left];
		rule->prob=new_prob<this->thresholdprob ? this->thresholdprob : new_prob;
		rule->expect_count=0;
	}

	//update the phrase rules;
	Phrase_table::iterator ephrase_iter;
	Phrase_entry::iterator cphrase_iter;
	double total_count=this->N_cnt[this->C];

	if(vb) //if variational bayes
	{
		double alpha= ITG::Config::GetSingleton().configfile->read<double>("alpha");
		for(ephrase_iter=this->phrase_table.begin();ephrase_iter!=this->phrase_table.end();ephrase_iter++)
		{
			for(cphrase_iter=ephrase_iter->second.begin();cphrase_iter!=ephrase_iter->second.end();cphrase_iter++)
			{
				double expect_count=cphrase_iter->second->expect_count;
		//		cout<< "old: "<<expect_count;
				if(expect_count+alpha>0)
				{
					double new_expect_count=exp(Tools::digamma(expect_count+alpha));
          //          cout<<"new: "<<new_expect_count<<endl;
					cphrase_iter->second->expect_count=new_expect_count;
				}
                else
                {
                    cout<<"wrong"<<endl;
                }
			}
		}

		//total_count=exp(Tools::digamma(total_count+this->phrase_rules_count*alpha));
        total_count=exp(Tools::digamma(total_count));

	}
	for(ephrase_iter=this->phrase_table.begin();ephrase_iter!=this->phrase_table.end();ephrase_iter++)
	{
		for(cphrase_iter=ephrase_iter->second.begin();cphrase_iter!=ephrase_iter->second.end();cphrase_iter++)
		{
			double expect_count=cphrase_iter->second->expect_count;
			double newprob = 0;
			newprob=expect_count/total_count;
			
			cphrase_iter->second->prob= newprob<this->thresholdprob ? this->defaultprob : newprob ;
			cphrase_iter->second->expect_count=0;

			if(this->debug_level>=3)
			{
				fprintf(Loger::logfile,"en: ");
				fprint_array(Loger::logfile,ephrase_iter->first);
				fprintf(Loger::logfile,"\n");
				fprintf(Loger::logfile,"cn: ");
				fprint_array(Loger::logfile,cphrase_iter->first);
				fprintf(Loger::logfile,"\n");
				fprintf(Loger::logfile,"%.2g,%.2g, =%.2g",expect_count,total_count,newprob);
				fprintf(Loger::logfile,"\n");
			}
		}

	}

	for(int i=0;i<this->nonterminal_count;i++)
	{
		this->N_cnt[i]=0;
	}
}



/**
 * @brief  Initialize the non_terminal rules.
 */
void SCFG::InitSystemRules()
{
	vector<string> sys_rules;
	double proba=1.0/3;
	double probb=1.0/6;
	sys_rules.push_back(Tools::d2str(proba)+"	S # A");
	sys_rules.push_back(Tools::d2str(proba)+"	S # B");
	sys_rules.push_back(Tools::d2str(proba)+"	S # C");
	sys_rules.push_back(Tools::d2str(probb)+"	A # [ A B ]");
	sys_rules.push_back(Tools::d2str(probb)+"	A # [ B B ]");
	sys_rules.push_back(Tools::d2str(probb)+"	A # [ C B ]");
	sys_rules.push_back(Tools::d2str(probb)+"	A # [ A C ]");
	sys_rules.push_back(Tools::d2str(probb)+"	A # [ B C ]");
	sys_rules.push_back(Tools::d2str(probb)+"	A # [ C C ]");

	sys_rules.push_back(Tools::d2str(probb)+"	B # < A A >");
	sys_rules.push_back(Tools::d2str(probb)+"	B # < B A >");
	sys_rules.push_back(Tools::d2str(probb)+"	B # < C A >");
	sys_rules.push_back(Tools::d2str(probb)+"	B # < A C >");
	sys_rules.push_back(Tools::d2str(probb)+"	B # < B C >");
	sys_rules.push_back(Tools::d2str(probb)+"	B # < C C >");

	this->AddSystemRules(sys_rules);
}



/**
 * @brief  Add the non_terminal rules to the grammar.
 *
 * @param sys_rules
 */
void SCFG::AddSystemRules(vector<string>& sys_rules)
{
	for(unsigned int i=0;i<sys_rules.size();i++)
	{
		Rule* rule=new Rule(sys_rules[i],i);
		if(rule->type == unary)
		{
			this->unary_rules.push_back(rule);
		    long long key=Tools::getHashkey(rule->left,rule->right1,0,rule->type);
			this->unary_rules_map[key]=rule;
		}
		else
		{
			this->binary_rules.push_back(rule);
			long long key=Tools::getHashkey(rule->left,rule->right1,rule->right2,rule->type);
			this->binary_rules_map[key]=rule;
		}
	}

	this->unary_rules_count=this->unary_rules.size();
	this->binary_rules_count=this->binary_rules.size();
	cout<<"set system rules ok!"<<endl;
}

//we have change this function, because of the multi thread training ,we need to precreate all the rules,
//so ,in this function, we need't to create a new rule ,we just query.
Rule* SCFG::GetPhrase2PhraseRule(vector<WORDTYPE> ephrase,vector<WORDTYPE> cphrase)
{
    vector<int> cphrase_tags;
    vector<int> ephrase_tags;
    PhraseRule* phrase_rule=NULL;

	Phrase_table::iterator ephrase_iter;
	Phrase_entry::iterator cphrase_iter;

	ephrase_iter=this->phrase_table.find(ephrase);
	if(ephrase_iter!=this->phrase_table.end())
	{
		cphrase_iter=ephrase_iter->second.find(cphrase);
		if(cphrase_iter!=ephrase_iter->second.end())
		{
			phrase_rule= cphrase_iter->second;
		}
    }
  
	return phrase_rule;
}



/**
 * @brief   Search and get the binary rule.
 *
 * @param lhs  Left hand side of the rule
 * @param rhsl The first right hand side of the rule.
 * @param rhsr The second right hand side of the rule.
 * @param type The rule's type. Straight rule or inverted rule.
 *
 * @return  The binary rule lhs -> rhs1 rhsr type 
 */
Rule* SCFG::GetRule(int lhs,int rhsl,int rhsr,RuleType type)
{
	long long key=Tools::getHashkey(lhs,rhsl,rhsr,type);
	std::unordered_map<long long ,Rule*>::iterator it = binary_rules_map.find(key);
	if(it!=binary_rules_map.end())
	{
		return it->second;
	}
	return NULL;
}



/**
 * @brief Search and get the unary rule.
 *
 * @param lhs The left hand side of the rule.
 * @param rhs The right hand side of the rule.
 * 
 * @return  The unary rule lhs -> rhs.
 */
Rule* SCFG::GetRule(int lhs,int rhs)
{
	long long key=Tools::getHashkey(lhs,rhs,0,unary);
	std::unordered_map<long long ,Rule*>::iterator it=unary_rules_map.find(key);
	if(it!=unary_rules_map.end())
	{
		return it->second;
	}
	return NULL;
}




/**
 * @brief It is not used now.
 *
 * @param senpair_index
 * @param estart
 * @param eend
 * @param cstart
 * @param cend
 * @param ephrase
 * @param cphrase
 *
 * @return 
 */
bool SCFG::IsPhraseSpan(int senpair_index,int estart,int eend,int cstart,int cend,WORDTYPE* ephrase,WORDTYPE* cphrase)
{
	vector<int> c_e= prior_knowledge->Model4_Intersection()[senpair_index][0];
	vector<int> e_c= prior_knowledge->Model4_Intersection()[senpair_index][1];

	int link_count=0;
	
	while(*ephrase)
	{
		fprintf(Loger::logfile,"%s ",this->corpus.edict_reverse[*ephrase].c_str());
		ephrase++;
	}
	fprintf(Loger::logfile,"%s"," - ");
	while(*cphrase)
	{
		fprintf(Loger::logfile,"%s ",this->corpus.cdict_reverse[*cphrase].c_str());
		cphrase++;
	}
	fprintf(Loger::logfile,"\n");
	for(int i=estart;i<eend;i++)
	{
		if(e_c.at(i)>=0 )
		{
			if( e_c.at(i)<cstart || e_c.at(i)>=cend)
			{
				return false;
			}
			else
			{
				link_count++;
			}
		}
	}

	for(int i=cstart;i<cend;i++)
	{
		if(c_e.at(i)>=0 )
		{
			if( c_e.at(i)<estart || c_e.at(i)>=eend)
			{
				return false;
			}
			else
			{
				link_count++;
			}
		}
	}
	if(link_count==2)
	{
		fprintf(Loger::logfile,"%s\n","true");
		return true;
	}
	else
	{
		return false;
	}
		
}


// we need to adjust the original phrase's probability ,because we add some phrase rules
void SCFG::AdjustPhraseProb()
{
	double prob_normalize = ITG::Config::GetSingleton().configfile->read<double>("prob_normalize");
	Phrase_table::iterator ephrase_iter;
	Phrase_entry::iterator cphrase_iter;
	for(ephrase_iter=this->phrase_table.begin();ephrase_iter!=this->phrase_table.end();ephrase_iter++)
	{
		for(cphrase_iter=ephrase_iter->second.begin();cphrase_iter!=ephrase_iter->second.end();cphrase_iter++)
		{
			double old = cphrase_iter->second->prob;
			cphrase_iter->second->prob=prob_normalize*old;
		}
	}
}



/**
 * @brief The M-step for the feature ITG.
 *
 * @param weight  The feature weight.
 */
void SCFG::Update(vector<double>& weight)
{
	//update the unary rules
	for(int i=0;i<this->unary_rules_count;i++)
	{
		Rule* rule=this->unary_rules[i];
		double new_prob=rule->expect_count/this->N_cnt[rule->left];
		rule->prob= new_prob<this->thresholdprob ? this->thresholdprob : new_prob;
		rule->expect_count=0;
	}
	//update the bianry rules
	for(int i=0;i<this->binary_rules_count;i++)
	{
		Rule* rule=this->binary_rules[i];
		double new_prob=rule->expect_count/this->N_cnt[rule->left];
		rule->prob=new_prob<this->thresholdprob ? this->thresholdprob : new_prob;
		rule->expect_count=0;
	}

	Features& feature = Features::GetSingleton();
	Phrase_table::iterator ephrase_iter;
	Phrase_entry::iterator cphrase_iter;
	double sum_inner_product=0;
	vector<double> rules_sum;
	rules_sum.resize(this->phrase_rules_count);
	for(ephrase_iter=this->phrase_table.begin();ephrase_iter!= this->phrase_table.end();ephrase_iter++)
	{
		for(cphrase_iter=ephrase_iter->second.begin();cphrase_iter!=ephrase_iter->second.end();cphrase_iter++)
		{
			PhraseRule* rule = cphrase_iter->second;
			vector<pair<long long ,double> > rule_features = feature.rules_features_index_values[rule->rule_id];
			double rule_sum=0;
			for(unsigned int i=0;i< rule_features.size();i++)
			{
				long long index = rule_features[i].first ;
				double value = rule_features[i].second;
           //     if(weight[index]>0)
                {
			    	rule_sum+= weight[index]*value;
                }
			}
            
			rules_sum[rule->rule_id]=rule_sum;
           // cout<<rule->rule_id<<" "<<rule_sum<<endl;
            if (rule_sum>0)
            {
		        sum_inner_product+=exp(rule_sum);
            }

		}
	}
	for(ephrase_iter=this->phrase_table.begin();ephrase_iter!= this->phrase_table.end();ephrase_iter++)
	{
		for(cphrase_iter=ephrase_iter->second.begin();cphrase_iter!=ephrase_iter->second.end();cphrase_iter++)
		{
			PhraseRule* rule = cphrase_iter->second;
            double newprob= 0;
            if (rules_sum[rule->rule_id]>0)
            {
               newprob= exp(rules_sum[rule->rule_id])/sum_inner_product;
            }
			cphrase_iter->second->prob= newprob<this->thresholdprob ? this->defaultprob : newprob ;
			cphrase_iter->second->expect_count=0;
            if(use_pos_feature)
            {
                rule->ClearTags();
            }
		}
	}
	for(int i=0;i<this->nonterminal_count;i++)
	{
		this->N_cnt[i]=0;
	}

}


/**
 * @brief Compute the translation probability of the terminal rule. It's used for human readable grammar.
 */
void SCFG::ComputeTransProb()
{
	Phrase_table::iterator ephrase_iter;
	Phrase_entry::iterator cphrase_iter;
	for(ephrase_iter=this->phrase_table.begin();ephrase_iter!=this->phrase_table.end();ephrase_iter++)
	{
		double count=0;
		for(cphrase_iter=ephrase_iter->second.begin();cphrase_iter!=ephrase_iter->second.end();cphrase_iter++)
		{
			count+=cphrase_iter->second->prob;
		}

		
		for(cphrase_iter=ephrase_iter->second.begin();cphrase_iter!=ephrase_iter->second.end();cphrase_iter++)
		{
			double tran_prob=cphrase_iter->second->prob/count;
			cphrase_iter->second->tran_prob = tran_prob;
		}
	}
}


//we found that use the translation probability as the threshold is not appropriate,
//we use the rule probability as the threshold
double SCFG::ComputeThresholdProb()
{
	vector<double> rule_probs;
	rule_probs.resize(this->phrase_rules_count);
	Phrase_table::iterator ephrase_iter;
	Phrase_entry::iterator cphrase_iter;
	for(ephrase_iter=this->phrase_table.begin();ephrase_iter!=this->phrase_table.end();ephrase_iter++)
	{
		for(cphrase_iter=ephrase_iter->second.begin();cphrase_iter!=ephrase_iter->second.end();cphrase_iter++)
		{
			rule_probs[cphrase_iter->second->rule_id]= cphrase_iter->second->prob; // prob is propotion to the expect count;
		}
	}
	double rule_prob_threshold = ITG::Config::GetSingleton().configfile->read<double>("rule_prob_threshold");
	int index = rule_prob_threshold* this->phrase_rules_count;
	sort(rule_probs.begin(),rule_probs.end());
	return rule_probs[index];
}



/**
 * @brief  Because we use multiple threads to train the model. We need to create all the rule before the training stage.
 */
void SCFG::InitPhraseRules()
{
    cout<<"begin to create all the one-to-one phrase rules"<<endl; 
    cout<<"sentence pair count: "<<corpus.paircount<<endl;
    int senpair_start= ITG::Config::GetSingleton().configfile->read<int>("senpair_start");
    int senpair_end= ITG::Config::GetSingleton().configfile->read<int>("senpair_end");
    senpair_start = senpair_start>0 ? senpair_start : 0;
    senpair_end = senpair_end < corpus.paircount ? senpair_end : corpus.paircount;
    for(int i=senpair_start;i<senpair_end;i++)
    {
        SenPair* p_sentence_pair=corpus.senpairs[i];
        int elen=p_sentence_pair->GetElen();
        int clen=p_sentence_pair->GetClen();

        for(int espan=0;espan<=1;espan++)
        {
            for(int estart=0;estart<=elen-espan;estart++)
            {
                int eend=estart+espan;
                vector<WORDTYPE> ephrase;
                p_sentence_pair->GetEPhrase(estart,eend,ephrase);
                
                for(int cspan=0;cspan<=1;cspan++)
                {
                    if(espan==0 && cspan==0)
                        continue;

                    for(int cstart=0;cstart<=clen-cspan;cstart++)
                    {
                        int cend=cstart+cspan;
                        vector<WORDTYPE> cphrase;
                        p_sentence_pair->GetCPhrase(cstart,cend,cphrase);
                        this->PhraseRule_QueryAndCreate(i,estart,eend,cstart,cend,ephrase,cphrase);
                    }
                }
            }
        }
        if(i%100==0)
        {
            cout<<i<<'\r';
        }
    }
    cout<<"create phrase rules ok."<<endl;
    cout<<"total phrase rules count is: "<<this->phrase_rules_count<<endl;
}


/**
 * @brief Give me a ephrase , cphrase , I query if the rule has been in the grammar, if not, I create it.
 *
 * @param ephrase  english side phrase
 * @param cphrase  Chinese side phrase
 */
void SCFG::PhraseRule_QueryAndCreate( int sentence_id,int estart,int eend,int cstart,int cend,vector<WORDTYPE> ephrase,vector<WORDTYPE> cphrase)
{
    PhraseRule* phrase_rule=NULL;
	Phrase_table::iterator ephrase_iter;
	Phrase_entry::iterator cphrase_iter;

	ephrase_iter=this->phrase_table.find(ephrase);
	if(ephrase_iter!=this->phrase_table.end())
	{
		cphrase_iter=ephrase_iter->second.find(cphrase);
		if(cphrase_iter!=ephrase_iter->second.end())
		{
			phrase_rule= cphrase_iter->second;
		}
    }
    if(!phrase_rule)
    {
        phrase_rule=new PhraseRule(this->phrase_rules_count,1e-10,ephrase,cphrase);
        if(init_em_parameters)
        {
            double init_prob =prior_knowledge->GetRuleInitProb(*phrase_rule);
            if(init_prob>0)
            {
                phrase_rule->prob=init_prob;
            }   
        }
        phrase_table[ephrase][cphrase]=phrase_rule;
        this->phrase_rules.push_back(phrase_rule);
        this->phrase_rules_count++;
        
    }

    // we need call AddTags many times, because the same rule in different sentence maybe have different tags. we need collect this
    // statistics information 
    if(use_pos_feature)
    {
        vector<int> cphrase_tags;
        vector<int> ephrase_tags;
        prior_knowledge->GetCPhraseTags(sentence_id,cstart,cend,cphrase_tags);
        prior_knowledge->GetEPhraseTags(sentence_id,estart,eend,ephrase_tags);
        phrase_rule->AddTags(cphrase_tags,ephrase_tags);

        if(this->debug_level>=3)
        {
            cout<< phrase_rule->StrFormat()<<endl;
            for(unsigned int i=0;i<ephrase_tags.size();i++)
                cout<<ephrase_tags[i]<<" ";
            cout<<" | ";
            for(unsigned int i=0;i< cphrase_tags.size();i++)
                cout<<cphrase_tags[i]<<" ";

            cout<<endl;
        }
    }
}



/**
 * @brief Collect rule's expect count from multiple thread training result.
 *
 * @param params 
 */
void SCFG::Reduce_SCFG_Params(const vector<Train_Param*>& params)
{
    //we sum all the expect count to the first vector
    for(unsigned int i=1;i<params.size();i++)
    {
        for(unsigned int j=0;j<MAXN;j++)
        {
            params[0]->nonterminal_expect_count[j]+=params[i]->nonterminal_expect_count[j];
        }
        for(int j=0;j<this->unary_rules_count+this->binary_rules_count;j++)
        {
            params[0]->system_rules_expect_count[j]+=params[i]->system_rules_expect_count[j];
        }
        for(int j=0;j<this->phrase_rules_count;j++)
        {   
            params[0]->phrase_rules_expect_count[j]+=params[i]->phrase_rules_expect_count[j];
        }
    }
    
    //we update all the rules prob
    for(int i=0;i<this->unary_rules_count;i++)
    {
        Rule* rule = this->unary_rules[i];
        rule->expect_count=params[0]->system_rules_expect_count[rule->rule_id];
    }
    for(int i=0;i<this->binary_rules_count;i++)
    {
        Rule* rule =this->binary_rules[i];
        rule->expect_count=params[0]->system_rules_expect_count[rule->rule_id];
    }
    for(int i=0;i<this->phrase_rules_count;i++)
    {
        PhraseRule* rule=this->phrase_rules[i];
        rule->expect_count=params[0]->phrase_rules_expect_count[rule->rule_id];
    }

    for(int i=0;i<MAXN;i++)
    {
        this->N_cnt[i]=params[0]->nonterminal_expect_count[i];        
    }
}

bool  phrase_rule_cmp(PhraseRule* rule1,PhraseRule* rule2)
{
    return rule1->expect_count<rule2->expect_count;
}




/**
 * @brief  this method is not used now.
 *
 * @param threshold
 */
void SCFG::Shrink(double threshold)
{
    vector<PhraseRule*> tmp_phrase_rules;
    cout<<"before shrink: "<<this->phrase_rules_count<<endl;

    tmp_phrase_rules.resize(this->phrase_rules_count,0);
    for(int i=0;i<this->phrase_rules_count;i++)
    {
        tmp_phrase_rules[i]=this->phrase_rules[i];
    }
    sort(tmp_phrase_rules.begin(),tmp_phrase_rules.end(),phrase_rule_cmp);

//    cout<<"shrink"<<endl;
//    cout<<tmp_phrase_rules[0]->expect_count<<endl;
//    cout<<tmp_phrase_rules[1]->expect_count<<endl;
    int index=threshold*this->phrase_rules_count;
    double threshold_expect_count=tmp_phrase_rules[index]->expect_count;

    this->phrase_rules.clear();
    this->phrase_table.clear();
    this->N_cnt[3]=0;
    int new_phrase_id = 0;
    for(int i=0;i<this->phrase_rules_count;i++)
    {
        PhraseRule* phrase_rule = tmp_phrase_rules[i];
        if(phrase_rule->expect_count > threshold_expect_count)
        {
            phrase_rule->rule_id=new_phrase_id;
            this->N_cnt[3]+=phrase_rule->expect_count;
            new_phrase_id++;
            this->phrase_rules.push_back(phrase_rule);
            this->phrase_table[phrase_rule->m_ephrase][phrase_rule->m_cphrase]=phrase_rule;
        }
        else
        {
            delete phrase_rule;
        }
    }

    this->phrase_rules_count=new_phrase_id;
    cout<<"after shrink: "<<this->phrase_rules_count<<endl;
}
