#ifndef Cell_Rule

#define Cell_Rule

#include "rule.h"



class CellElem

{

public:

	CellElem(Rule* r,vector<int> index,double prob)
	{

		this->rule=r;

		this->children_index=index;

		this->in_prob =prob;

	}



	~CellElem()
	{

	};

	Rule* rule;

	vector<int> children_index;

	double in_prob;

};

#endif