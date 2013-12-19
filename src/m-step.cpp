#include "m-step.h"
#include "scfg.h"
#include <cassert>
#include <cmath>
#include "config.h"
#include "align_features.h"
#include <numeric>
#include <iomanip>

void PreComputeLBFGS_Param(LBFGS_Param* lbfgs_param,int n)
{
    Features& feature=Features::GetSingleton();
    double sum_expect_count=0;     
   
    lbfgs_param->sum_expect_feature_value.clear();
    lbfgs_param->sum_expect_feature_value.resize(n,0);

    for(int i=0;i<lbfgs_param->scfg->phrase_rules_count;i++)
    {
        PhraseRule* rule=lbfgs_param->scfg->phrase_rules[i];
       
        //cout<< rule->StrFormat()<< "   expect count: "<<rule->expect_count<<endl;

        sum_expect_count+=rule->expect_count;
        
        const vector<pair<long long,double> >& rule_features= feature.rules_features_index_values[i];
        for(unsigned int j=0;j<rule_features.size();j++)
        {
            long long feature_index = rule_features[j].first;
            double feature_value= rule_features[j].second;
            lbfgs_param->sum_expect_feature_value[feature_index]+=rule->expect_count*feature_value;
        }
    }
    lbfgs_param->sum_expect_count=sum_expect_count;
}
/* compute the value of the object function */
lbfgsfloatval_t evaluate_function_value_jp( void *instance, int n, const lbfgsfloatval_t *x)
{
	SCFG* scfg =(SCFG*)instance;
	double sum_exp=0;
	double log_sum_exp=0;
	Features& feature =Features::GetSingleton();
	vector<double> rule_sums;
	rule_sums.resize(scfg->phrase_rules_count);

    for(int i=0;i<scfg->phrase_rules_count;i++)
    {
        vector<pair<long long,double> >* rule_features = &(feature.rules_features_index_values[i]);
        double rule_sum=0;
        for(unsigned int j=0;j<rule_features->size();j++)
        {
            long long feature_index=(*rule_features)[j].first;
            double feature_value=(*rule_features)[j].second;
            rule_sum+=x[feature_index]*feature_value;
        }
        rule_sums[i]=rule_sum;
        sum_exp+=exp(rule_sum);
    }

	log_sum_exp=log(sum_exp);
	double function_value=0;

    for(int i=0;i<scfg->phrase_rules_count;i++)
    {
        PhraseRule* phrase_rule=scfg->phrase_rules[i];
        double diff=phrase_rule->expect_count*(rule_sums[i]-log_sum_exp);
        function_value+=diff;
    }

	double penalty=0;
	for(int i=0;i<n;i++)
	{
		penalty+= (x[i]*x[i]);
	}
	double L2_coefficient = ITG::Config::GetSingleton().configfile->read<double>("L2_coefficient"); 
	penalty= penalty*L2_coefficient;
    double fx = function_value-penalty;

	return -fx; // the lbfgs minimize the function ,so we return the negative value;
}

// compute the gradient of the object function under the current parameters */
void evaluate_function_gradient_jp ( void *instance, const lbfgsfloatval_t *x ,lbfgsfloatval_t *g, const int n, const lbfgsfloatval_t step)
{
	double L2_coefficient = ITG::Config::GetSingleton().configfile->read<double>("L2_coefficient"); 
	SCFG* scfg =(SCFG*)instance;
	double sum_exp=0;
	Features& feature =Features::GetSingleton();
	vector<double> rule_scores;
	rule_scores.resize(scfg->phrase_rules_count);

	//compute sum_{e',f'} exp<lambda,f(e',f')>
    for(int i=0;i<scfg->phrase_rules_count;i++)
    {
        vector<pair<long long,double> >* rule_features = &(feature.rules_features_index_values[i]);
        double rule_score=0;
        for(unsigned int j=0;j<rule_features->size();j++)
        {
            long long feature_index=(*rule_features)[j].first;
            double feature_value=(*rule_features)[j].second;
            rule_score+=x[feature_index]*feature_value;
        }
        rule_scores[i]=rule_score;
        sum_exp+=exp(rule_score);
    }
    
	// compute sum{e',f'} P(C->e'|f')*f(e',f') and sum{e',f'} e_{tr}*f(e,f)
	vector<double> sum_expect_feature_value;
	sum_expect_feature_value.resize(n,0);
    vector<double> sum_expect_prob_feature_value;
    sum_expect_prob_feature_value.resize(n,0);
    double sum_expect_count=0;

    for(int i=0;i<scfg->phrase_rules_count;i++)
    {
        double prob=exp(rule_scores[i])/sum_exp;
        PhraseRule* phrase_rule=scfg->phrase_rules[i];
        vector<pair<long long ,double> >* rule_features=&(feature.rules_features_index_values[i]);
        for(unsigned int j=0;j<rule_features->size();j++)
        {
            long long feature_index=(*rule_features)[j].first;
            double feature_value=(*rule_features)[j].second;
            sum_expect_prob_feature_value[feature_index]+=prob*feature_value;
            sum_expect_feature_value[feature_index]+=phrase_rule->expect_count*feature_value;
        }
        sum_expect_count+=phrase_rule->expect_count;
    }

	//compute the gradient = sum_{tr} e_{tr}* (sum{e',f'} P(C->e'|f')*f(e',f')) - sum_{tr} e_{tr}*f(e,f)
	for(int i=0;i<n;i++)
	{
		g[i]= sum_expect_count*sum_expect_prob_feature_value[i]-sum_expect_feature_value[i]+2*L2_coefficient*x[i];
	}

}

//we use one evaluate function to gain the speed
lbfgsfloatval_t evaluate_main(void* instance,const lbfgsfloatval_t* x,lbfgsfloatval_t* g,const int n,const lbfgsfloatval_t step)
{
    int debug_level = ITG::Config::GetSingleton().configfile->read<int>("debug_level");
    LBFGS_Param* lbfgs_param=(LBFGS_Param*)instance;
    SCFG* scfg=lbfgs_param->scfg;
    double L2_coefficient=lbfgs_param->L2_coefficient;
	double sum_exp=0;
	double log_sum_exp=0;
	Features& feature =Features::GetSingleton();
	vector<double> rule_scores;
	rule_scores.resize(scfg->phrase_rules_count,0);

    for(int i=0;i<scfg->phrase_rules_count;i++)
    {
        PhraseRule* phrase_rule=scfg->phrase_rules[i];
       // cout<<phrase_rule->StrFormat()<<"   expect count: "<<phrase_rule->expect_count<<endl;

        const vector<pair<long long,double> >& rule_features = feature.rules_features_index_values[i];
        double rule_score=0;
        for(unsigned int j=0;j<rule_features.size();j++)
        {
            long long feature_index=rule_features[j].first;
            double feature_value=rule_features[j].second;
            rule_score+=x[feature_index]*feature_value;

            if(debug_level>=1)
            {
                if(  feature_value>0 && x[feature_index]!= 0)
                {
                    cout<<"f index "<<j<<" "<<setw(-30)<<phrase_rule->StrFormat()<<"\t expect count: "<<setprecision(5)<<phrase_rule->expect_count<<"\tfeature value: "<<feature_value<<"\tfeature weight: "<<setprecision(8)<<x[feature_index]<<endl;
                }
            }
        }

        if(debug_level>=1)
        {
            cout<<"rule_score: "<<rule_score<<endl<<endl;
        }
        rule_scores[i]=rule_score;
        sum_exp+=exp(rule_score);
    }
    

	log_sum_exp=log(sum_exp);
	double function_value=0;
    vector<double> sum_expect_prob_feature_value(n,0);
    double sum_expect_count=lbfgs_param->sum_expect_count;


    for(int i=0;i<scfg->phrase_rules_count;i++)
    {
        PhraseRule* phrase_rule=scfg->phrase_rules[i];
        double diff=phrase_rule->expect_count*(rule_scores[i]-log_sum_exp);
        function_value+=diff;

        // compute sum{e',f'} P(C->e'|f')*f(e',f')
        double prob=exp(rule_scores[i])/sum_exp;
        const vector<pair<long long ,double> >&  rule_features= feature.rules_features_index_values[i];
        for(unsigned int j=0;j<rule_features.size();j++)
        {
            long long feature_index=rule_features[j].first;
            double feature_value= rule_features[j].second;
            sum_expect_prob_feature_value[feature_index]+=prob*feature_value;
        }
    }

	//compute the gradient = sum_{tr} e_{tr}* (sum{e',f'} P(C->e'|f')*f(e',f')) - sum_{tr} e_{tr}*f(e,f)
	for(int i=0;i<n;i++)
	{
		g[i]= sum_expect_count*sum_expect_prob_feature_value[i]-lbfgs_param->sum_expect_feature_value[i]+2*L2_coefficient*x[i];
	}

    double penalty=0;
    if(L2_coefficient>0)
    {
        penalty = accumulate(x, x+n, 0.0);
        //for(int i=0;i<n;i++)
    	//{
	    //	penalty+= (x[i]*x[i]);
    	//}
    }	
	penalty= penalty*L2_coefficient;
    double fx = function_value-penalty;

    return -fx;



}
lbfgsfloatval_t evaluate(
    void *instance,
    const lbfgsfloatval_t *x,
    lbfgsfloatval_t *g,
    const int n,
    const lbfgsfloatval_t step
    )
{
    return 0;
}

int progress(
    void *instance,
    const lbfgsfloatval_t *x,
    const lbfgsfloatval_t *g,
    const lbfgsfloatval_t fx,
    const lbfgsfloatval_t xnorm,
    const lbfgsfloatval_t gnorm,
    const lbfgsfloatval_t step,
    int n,
    int k,
    int ls
    )
{
    printf("Iteration %d:\n", k);
    printf("  fx = %f, x[0] = %f, x[1] = %f\n", fx, x[0], x[1]);
    printf("  xnorm = %f, gnorm = %f, step = %f\n", xnorm, gnorm, step);
    printf("\n");
    return 0;
}

