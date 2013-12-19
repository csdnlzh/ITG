#ifndef Train_Param_H
#define Train_Param_H
#include <vector>
using namespace std;

class Train_Param
{
public:
    Train_Param(int nonterminal_count,int system_rules_count,int phrase_rules_count)
    {
        nonterminal_expect_count.resize(nonterminal_count,0);
        system_rules_expect_count.resize(system_rules_count,0);
        phrase_rules_expect_count.resize(phrase_rules_count,0);
    }
    vector<double> nonterminal_expect_count;
    vector<double> system_rules_expect_count;
    vector<double> phrase_rules_expect_count;
};
#endif
