#include <fstream>
#include <sstream>
#include "pruning.h"
#include "config.h"
#include "rule.h"
#include "prior_knowledge.h"

void Prune::Use_IBMmode()
{
	
}

vector<vector<vector<int> > >&  Prune::Fixed_link()
{
	return Prior_Knowledge::GetInstance().Model4_Intersection();
}
