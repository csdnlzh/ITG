#ifndef Pruning_H
#define Pruning_H
#include <vector>
#include "senpair.h"
using namespace std;

class Prune
{
public:
	Prune()
	{

	}
	//use IBM  model's translation table
	void Use_IBMmode();

	vector<vector<vector<int> > >& Fixed_link();
};
#endif