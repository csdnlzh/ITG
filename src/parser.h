#ifndef Parser_H
#define Parser_H
#include "senpair.h"
#include "scfg.h"
#include "table_cell.h"
#include "rule.h"
#include "pruning.h"
class Parser
{
public:
	Parser(SCFG& scfg):grammar(scfg)
	{
		config=&(ITG::Config::GetSingleton());
		this->pruning=config->configfile->read<int>("pruning")==1 ? true : false; 
	
		conflict_link = config->configfile->read<int>("conflict_link");
        this->multi_words_rules_count=0;
        prune_count=0;
	}
    int multi_words_rules_count;
    int prune_count;
	ParseTableCell*  Parse(SenPair* senpair);
private:
    ITG::Config* config;
	bool pruning;
	SCFG& grammar;
	SenPair* senpair;
	int elen;
	int clen;
	int e_dim;
	int c_dim;
	int e_dim_c_dim_c_dim;
	int c_dim_c_dim;
	ParseTableCell*  ptable;
	int* pindicate_table;
   //	vector<vector<vector<int> > > senpairs_links;
	int conflict_link;
	void InitTable();
	void ChartParse();
	void Increase_inside(int s,int t,int u,int v,int lhs,double prob,vector<int>& children_index,Rule* rule);
	double Get_inside_prob(int s,int t,int u,int v,int lhs);

	inline int GetArrayIndex(int s,int t,int u,int v)
	{
		return s*(e_dim_c_dim_c_dim)+t*(c_dim_c_dim)+u*c_dim+v;
	}
	void InitIndicateTable();
	void ReleaseTable();
	void Pruning(); 
};

#endif
