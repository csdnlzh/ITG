#ifndef Trainer_H

#define Trainer_H

#include <vector>
#include "lbfgs.h"
#include "senpair.h"
#include "scfg.h"
#include "table_cell.h"
#include "config.h"
#include "pruning.h"
#include "train_param.h"

using namespace std;

typedef TableCell* TableCellPtr;

class Trainer
{

public:

	Trainer(int id,vector<SenPair*>& ps,SCFG& scfg,int senpair_start,int senpair_end,Train_Param& train_param,vector<double>& total_log_prob):pairs(ps),grammar(scfg),m_train_param(train_param),m_total_log_prob(total_log_prob)
	{
        this->trainer_id=id;
		config=&(ITG::Config::GetSingleton());
		this->debug_level=config->configfile->read<int>("debug_level");
		this->pruning=config->configfile->read<int>("pruning")==1 ? true : false; 
		if(pruning)
		{
			Prune prune;
			this->senpairs_links=prune.Fixed_link();
		}
		
		conflict_link = config->configfile->read<int>("conflict_link");
		this->prunned_span_count=0;
		this->total_span_count=0;
		this->senpair_start = senpair_start;
        this->senpair_end = senpair_end;
		this->vb=config->configfile->read<int>("vb")==1 ? true :false;
	}

	~Trainer()
	{
	}
    
	void operator()();

	void Pruning(); 

	//use dynamic programming to do pruning
	void PruningDP();

	void Inside();

	void OutSide();

	void ExpectCount();

	void InitIndicateTable();

	void InitIOTable();

	void ReleaseTable();

	double Compute_SentencProb();

	void Increase_inside(int s,int t,int u,int v,int lhs,double prob);
	void Increase_inside(int index,int lhs,double prob);//for efficient
	void Increase_outside(int s,int t,int u,int v,int lhs,double prob);
	void Increase_outside(int index,int lhs,double prob);//for efficient
	double Get_inside_prob(int s,int t,int u,int v,int lhs);
	double Get_inside_prob(int index ,int lhs); //for efficient
	double Get_outside_prob(int s,int t, int u,int v,int lhs);
	double Get_outside_prob(int index,int lhs);//for efficient

	void OptimizeFeatureWeights_berkeley(vector<double>& weights);
	void OptimizeFeatureWeights_jp(vector<double>& weights);
	void PrintOptimizeStat(vector<double>& weights);
private:

    // the identity of the trainer 
    int trainer_id;
	//the table used for inside-outside 
	TableCell* ptable;
	
	//the table used for pruning, indicate whether we conside the span
	int* pindicate_table;

	vector<SenPair*>& pairs;
	SenPair* training_pair;
	SCFG& grammar;
    Train_Param& m_train_param;
    vector<double>& m_total_log_prob;
	int it;
    ITG::Config* config;
	int debug_level;
	int clen;  //chinese sentence length
	int elen;  //english sentence length
	int c_dim; //dimension of c
	int e_dim; //dimension of e

	vector<vector<vector<int> > > senpairs_links;
	int conflict_link;
	long long  prunned_span_count;
	long long  total_span_count;
	int senpair_end;
	int senpair_start;
	bool pruning;
	bool vb ; // variatinal bayes flag

	int e_dim_c_dim_c_dim;
	int c_dim_c_dim;
	inline int GetArrayIndex(int s,int t,int u,int v)
	{
		return s*(e_dim_c_dim_c_dim)+t*(c_dim_c_dim)+u*c_dim+v;
	}

    void DisplayTrainProcess(int row,int col);
};

class MStep_Trainer
{
public:
    MStep_Trainer(SCFG& scfg):m_scfg(scfg)
    {
        m_pconfig=&(ITG::Config::GetSingleton());
    }
    void OptimizeFeatureWeights_berkeley(vector<double>& weights);
	void OptimizeFeatureWeights_jp(vector<double>& weights);
	void WriteFeature_Value_Weights(const vector<double>& weights,const SCFG& scfg);
    void InitFeatureWeights_RuleProb(vector<double>& weights,const SCFG& scfg);
private:
    SCFG& m_scfg;
    ITG::Config* m_pconfig;
};
#endif

