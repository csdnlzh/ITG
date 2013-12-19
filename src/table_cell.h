#ifndef Table_Cell_H
#define Table_Cell_H
#include "rule.h"
#include "globals.h"
#include <vector>

class TableCell
{
public:

	TableCell()
	{
		for(int i=0;i<MAXN;i++)
		{
			in_prob_dict[i]=0;
			out_prob_dict[i]=0;
		}
	}
	double in_prob_dict[MAXN];
	double out_prob_dict[MAXN];
};

class ParseTableCell: public TableCell
{
public:
	ParseTableCell()
	{
		children_index.resize(MAXN);
		fill(rule,rule+MAXN,static_cast<Rule*>(NULL));
	}
	vector<vector<int> > children_index;

	Rule*  rule[MAXN];
};
#endif
