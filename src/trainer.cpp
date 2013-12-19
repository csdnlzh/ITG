#include <iostream>
#include <math.h>
#include <stdlib.h>
#include <curses.h>
#include <boost/algorithm/string.hpp>
#include "trainer.h"
#include "senpair.h"
#include "table_cell.h"
#include "config.h"
#include "loger.h"
#include "symbols.h"
#include "m-step.h"
#include "align_features.h"
#include "features_manager.h"




/**
 * @file trainer.cpp
 * @brief Training algorithm for traditional ITG and feature-based ITG
 * @author lizhonghua
 * @version 1.0
 * @date 2012-03-29
 */


/**
 * @brief The entry point for multi-thread training.
 */
void Trainer::operator()()
{

    this->prunned_span_count=0;
    this->total_span_count=0;
    int fail_count=0;
    double total_logprob=0.0;

    int total_count=senpair_end-senpair_start;
    int piece=total_count/50;

    int index=0;
    int has_done=0;
    for(int i=senpair_start;i<senpair_end;i++)
    {
        this->training_pair =this->pairs[i];
        this->clen=training_pair->GetClen();
        this->elen=training_pair->GetElen();
        this->c_dim = clen+1;
        this->e_dim = elen+1;
        e_dim_c_dim_c_dim= e_dim*c_dim*c_dim; // for efficient;
        c_dim_c_dim=c_dim*c_dim; // for efficient;

        this->InitIndicateTable();
        if(pruning)
        {
            this->Pruning();
        }
        this->InitIOTable();
        this->Inside();
        double sentence_prob=this->Compute_SentencProb();
        if(sentence_prob>0)
        {
            m_total_log_prob[this->trainer_id]+=log(sentence_prob);
            this->OutSide();
            this->ExpectCount();
            this->ReleaseTable();
        }
        else
        {
           // cout<<"parsing sentence failed "<<endl;
            fail_count++;
            this->ReleaseTable();
        }
        has_done++;
        if(has_done==piece)
        {
            has_done=0;
            this->DisplayTrainProcess(this->trainer_id+3,index);
            index++;
        }
    }

    //cout<<":  log prob: "<<total_logprob<<endl;
		Loger::LogTime();
		Loger::mylogfile<<"total log prob: \t"<<total_logprob<<endl;
		Loger::mylogfile<<"fail count: \t"<<fail_count<<endl;
		Loger::mylogfile<<"total span count: \t"<<this->total_span_count<<endl;
		Loger::mylogfile<<"prunned span count: \t"<<this->prunned_span_count<<endl;
		Loger::mylogfile<<"prunned span ration: \t"<<(double)this->prunned_span_count/(double)this->total_span_count<<endl<<endl;
		
}

/**
 * @brief Compute Pruning indicator table 
 */
void Trainer::Pruning()
{
	vector<int> c_e=this->senpairs_links[this->training_pair->GetID()][0];
	vector<int> e_c=this->senpairs_links[this->training_pair->GetID()][1];
	
	for(int estart=0;estart<elen;estart++)
	{
		for(int eend=estart+1;eend<=elen;eend++)
		{
			for(int cstart=0;cstart<clen;cstart++)
			{
				for(int cend=cstart+1;cend<=clen;cend++)
				{
					int index=this->GetArrayIndex(estart,eend,cstart,cend);
					//c-->e
					for(int i=cstart;i<cend;i++)  //i can't equal cend, cspan [0,3] only contain words c[0],c[1],c[2]
					{
						if(c_e.at(i)<0)
						{
							continue;
						}
						if( c_e.at(i)>=eend || c_e.at(i)<estart)  //when c_e[i]==eend ,that's a wrong link
						{
							pindicate_table[index]+=1;
						}
					}
					//e-->c
					for(int j=estart;j<eend;j++)
					{
						if(e_c.at(j)<0)
						{
							continue;
						}
						if(e_c.at(j)>=cend || e_c.at(j)<cstart)
						{
							pindicate_table[index]+=1;
						}
					}
					if(this->debug_level>2)
					{
						cout<<estart<<" "<<eend<<" "<<cstart<<" "<<cend<<" "<<(this->pindicate_table)[index]<<endl;
					}
				}
			}
		}
	}
}
double Trainer::Compute_SentencProb()
{
	return this->Get_inside_prob(0,elen,0,clen,0);
}
void Trainer::InitIOTable()
{
	int count=e_dim*e_dim*c_dim*c_dim;
	this->ptable = new TableCell[count];

	//init the table with phrase translation probability
	for(int espan=0;espan<=this->elen;espan++)
	{
		for(int estart=0;estart<=this->elen-espan;estart++)
		{
			int eend=estart+espan;
		    vector<WORDTYPE> ephrase;
			//get the ch phrase according to the ch span index
			this->training_pair->GetEPhrase(estart,eend,ephrase);

			for(int cspan=0;cspan<=this->clen;cspan++)
			{
				if(espan==0 && cspan==0)
					continue;

				for(int cstart=0;cstart<=this->clen-cspan;cstart++)
				{
					int cend=cstart+cspan;
					int index=this->GetArrayIndex(estart,eend,cstart,cend);

					if(pruning && pindicate_table[index]>this->conflict_link)
					{
						continue;
					}
					//get the eng phrase according to the eng span index
                    vector<WORDTYPE> cphrase;
					this->training_pair->GetCPhrase(cstart,cend,cphrase);

					//how to deal with if we can't find the proper rule ?
					Rule* rule = this->grammar.GetPhrase2PhraseRule(ephrase,cphrase);
					if(rule && rule->prob>0)
					{	
						this->Increase_inside(index,rule->left,rule->prob);

						if (this->debug_level >= 3)
						{
						  fprintf(Loger::logfile, 
							  "prob[%d,%d,%d,%d] = %.2g\n",estart,eend,cstart,cend,rule->prob);
						  fprintf(Loger::logfile, "  en = ");
						  fprint_array(Loger::logfile,ephrase);
						  fprintf(Loger::logfile, "\n");
						  fprintf(Loger::logfile, "  cn = ");
						  fprint_array(Loger::logfile, cphrase);
						  fprintf(Loger::logfile, "\n");
						  fflush(Loger::logfile);
						 }
					}
					
				}
			}
		    
		}
	}

	//init the out-side probability
	this->Increase_outside(0,elen,0,clen,grammar.start_symbol,1.0);
}

void Trainer::InitIndicateTable()
{
	int count=e_dim*e_dim*c_dim*c_dim;
	this->pindicate_table= new int[count];
	memset(this->pindicate_table,0,sizeof(int)*count);
}
void Trainer::ReleaseTable()
{
	delete [] this->ptable;
	delete [] this->pindicate_table;
}



/**
 * @brief The inside algorithm for E-step.  From down to up.
 */

void Trainer::Inside()
{

	for(int espan=0;espan<=this->elen;espan++)
	{
		for(int estart=0;estart<=this->elen-espan;estart++)
		{
			int eend=estart+espan;
			for(int cspan=0;cspan<=this->clen;cspan++)
			{
				if(espan==0 && cspan==0)
					continue;
				for(int cstart=0;cstart<=this->clen-cspan;cstart++)
				{
					int cend=cstart+cspan;
					int index=this->GetArrayIndex(estart,eend,cstart,cend);
					this->total_span_count++;
					if(pruning && pindicate_table[index] >this->conflict_link)
					{
						this->prunned_span_count++;
						continue;
					}

					//for every binary rules;
					for(int ruleno=0;ruleno<this->grammar.binary_rules_count;ruleno++)
					{
						Rule* rule =this->grammar.binary_rules[ruleno];
						int lhs=rule->left;
						int rhsl=rule->right1;
						int rhsr=rule->right2;

						for(int esplit=estart;esplit<=eend;esplit++)
						{
							for(int csplit=cstart;csplit<=cend;csplit++)
							{
								double betal=0;
								double betar=0;
								if(rule->type==bi_reverse)
								{
									betal=this->Get_inside_prob(estart,esplit,csplit,cend,rhsl);
									if(betal<=0.0) continue;
									betar=this->Get_inside_prob(esplit,eend,cstart,csplit,rhsr);
									if(betar<=0.0) continue;
                                    
                                    if(this->debug_level>=3)
                                    {
                                        cout<<"bi_reverse: estart esplit csplit cend esplit eend cstart csplit " <<estart<<esplit<<csplit<<cend<<esplit<<eend<<cstart<<csplit<<endl;
                                    }

								}
								else
								{
									betal=this->Get_inside_prob(estart,esplit,cstart,csplit,rhsl);
									if(betal<=0.0) continue;
									betar=this->Get_inside_prob(esplit,eend,csplit,cend,rhsr);
									if(betar<=0.0) continue;

                                    if(this->debug_level>=3)
                                    {
                                        cout<<"bi_same: estart esplit cstart csplit esplit eend csplit cend "<< estart<<esplit<<cstart<<csplit<<esplit<<eend<<csplit<<cend<<endl; 
                                    }
								}

								double mass=rule->prob*betal*betar;
								this->Increase_inside(index,lhs,mass);
								if(this->debug_level>= 3)
								{
									cout<<"inside prob: "<<Symbols::invert_symbols[lhs]<<Symbols::invert_symbols[rhsl]<<Symbols::invert_symbols[rhsr]<<"   ["<<estart<<" "<<eend<<"]  ["<<cstart<<" "<<cend<<"]  "<<mass<<" "<<rule->prob<<" "<<betal<<" "<<betar<<endl;
								}
							}
						}

					}

					//for every unary rules;
					for(int ruleno=0;ruleno<this->grammar.unary_rules_count;ruleno++)
					{
						Rule* rule=this->grammar.unary_rules[ruleno];
						int lhs=rule->left;
						int rhs=rule->right1;
						
						double inprob=this->Get_inside_prob(index,rhs);
						double mass=rule->prob*inprob;
						if(inprob>0)
						{
							this->Increase_inside(index,lhs,mass);
							if(debug_level>= 3)
							{
								cout<<"inside prob:  "<<Symbols::invert_symbols[lhs]<<Symbols::invert_symbols[rhs]<<"   ["<<estart<<" "<<eend<<"]  ["<<cstart<<" "<<cend<<"] "<<mass<<" "<<rule->prob<<" "<<inprob<<endl;
							}
						}
					}

					 if (this->debug_level >= 3) 
					 {
						for (int i=0; i<this->grammar.nonterminal_count; i++)
						{
						  fprintf(Loger::logfile, "  * inside prob of [%s;%d,%d,%d,%d] = %g\n", 
							 Symbols::invert_symbols[i].c_str(),
							 estart, eend, cstart, cend,  this->Get_inside_prob(index,i));
						}
					 }
				}//endof cstart
			}//endof cspan
		}//endof estart
	}//endof espan

    if(this->debug_level>=3)
    {
        cout<<"-------------------------------"<<endl;
    }
}


/**
 * @brief   compute the outside probability from up to down
*/
void Trainer::OutSide()
{
	for(int espan=elen;espan>0;espan--)
	{
		for(int estart=0;estart<=elen-espan;estart++)
		{
			int eend=estart+espan;
			for(int cspan=clen;cspan>0;cspan--)
			{
				for(int cstart=0;cstart<=clen-cspan;cstart++)
				{
					int cend=cstart+cspan;
					int index=this->GetArrayIndex(estart,eend,cstart,cend);
					if(pruning && pindicate_table[index] >this->conflict_link)
					{
						continue;
					}
					//for unary rules
					for(int ruleno=0;ruleno<this->grammar.unary_rules_count;ruleno++)
					{
						Rule* rule=this->grammar.unary_rules[ruleno];
						int lhs=rule->left;
						int rhs=rule->right1;
						double alpha=this->Get_outside_prob(index,lhs);
						if(alpha>0)
						{
							double mass=rule->prob*alpha;
							this->Increase_outside(index,rhs,mass);
						}
					}

					//for binary rules
					for(int ruleno=0;ruleno<this->grammar.binary_rules_count;ruleno++)
					{
						Rule* rule=this->grammar.binary_rules[ruleno];
						int lhs=rule->left;
						int rhsl=rule->right1;
						int rhsr=rule->right2;
						double ruleprob=rule->prob;
				
						double alpha=this->Get_outside_prob(index,lhs);
						if(alpha>0.0)
						{
							for(int esplit=estart;esplit<=eend;esplit++)
							{
								for(int csplit=cstart;csplit<=cend;csplit++)
								{
									if(rule->type==bi_reverse)
									{
										//the inside probability of the left child of the parent node
										double betal=this->Get_inside_prob(estart,esplit,csplit,cend,rhsl);
										if(betal<=0.0) continue;
										//add the right child outside probability
										this->Increase_outside(esplit,eend,cstart,csplit,rhsr,alpha*ruleprob*betal);

										//the inside probability of the right child of the parent node
										double betar=this->Get_inside_prob(esplit,eend,cstart,csplit,rhsr);
										if(betar<=0.0) continue;
										//add the left child outside probability
										this->Increase_outside(estart,esplit,csplit,cend,rhsl,alpha*ruleprob*betar);
									}
									else //same as above
									{
										double betal=this->Get_inside_prob(estart,esplit,cstart,csplit,rhsl);
										if(betal<=0.0) continue;
										this->Increase_outside(esplit,eend,csplit,cend,rhsr,alpha*ruleprob*betal);

										double betar=this->Get_inside_prob(esplit,eend,csplit,cend,rhsr);
										if(betar<=0.0) continue;
										this->Increase_outside(estart,esplit,cstart,csplit,rhsl,alpha*ruleprob*betar);
									}
								}
							}
						}
					}

					 if (this->debug_level >= 3) 
					 {
						for (int i=0; i<this->grammar.nonterminal_count; i++)
						{
						  fprintf(Loger::logfile, "  * outside prob of [%s;%d,%d,%d,%d] = %g\n", 
							 Symbols::invert_symbols[i].c_str(),
							 estart, eend, cstart, cend,  this->Get_outside_prob(index,i));
						}
					 }
				}//endof estart
			}//endof espan
		}//endof cstart
	}//endof cspan
}


/**
 * @brief After the inside and outside procedure. We use this function compute the Expect count of the rule.
 */
void Trainer::ExpectCount()
{
	double sentence_prob=this->Compute_SentencProb();

	for(int estart=0;estart<=elen;estart++)
	{
		for(int eend=estart;eend<=elen;eend++)
		{
            vector<WORDTYPE> ephrase;
			this->training_pair->GetEPhrase(estart,eend,ephrase);


			for(int cstart=0;cstart<=clen;cstart++)
			{
				for(int cend=cstart;cend<=clen;cend++)
				{
					if(cend-cstart==0 && eend-estart==0) continue;

					int index=this->GetArrayIndex(estart,eend,cstart,cend);
					if(pruning && pindicate_table[index] >this->conflict_link)
					{
						continue;
					}
                    vector<WORDTYPE> cphrase;
					this->training_pair->GetCPhrase(cstart,cend,cphrase);

					for(int lhs=0;lhs<this->grammar.nonterminal_count;lhs++)
					{
						double alpha=this->Get_outside_prob(index,lhs);
						if(alpha<=0.0) continue;
						double beta=this->Get_inside_prob(index,lhs);
						if(beta<=0.0) continue;

						double alb=alpha*beta;

						//if the rule type is phrase pair
						if(lhs==this->grammar.C)
						{
							Rule* phrase_rule=this->grammar.GetPhrase2PhraseRule(ephrase,cphrase);
							double rule_expect_count=alpha*phrase_rule->prob/sentence_prob;
                            
                            //modify for multi thread training 
							//phrase_rule->expect_count+=rule_expect_count;
                            this->m_train_param.phrase_rules_expect_count[phrase_rule->rule_id]+=rule_expect_count;
							//this->grammar.N_cnt[lhs]+=rule_expect_count;  //C -> e/f add all these count
                            this->m_train_param.nonterminal_expect_count[lhs]+=rule_expect_count;

							if (this->debug_level >= 3)
							{
								fprintf(Loger::logfile, "phrase pari - span = %d,%d,%d,%d\n", estart, eend, cstart, cend);
								fprintf(Loger::logfile, "** alpha = %g\n", alpha);
								fprintf(Loger::logfile, "** beta = %g\n", beta);
								fprintf(Loger::logfile, "** pi = %g\n", sentence_prob);
								fprintf(Loger::logfile, "** expect count = %g\n", rule_expect_count);
							}
						}
						else
						{
							//this->grammar.N_cnt[lhs]+=alb/sentence_prob;
							// rhs=0 is S ,S is not as a rhs for ever.
                            this->m_train_param.nonterminal_expect_count[lhs]+=alb/sentence_prob;

							for(int rhsl=0;rhsl<this->grammar.nonterminal_count;rhsl++)
							{
								//unary rule
								Rule* unary_rule=this->grammar.GetRule(lhs,rhsl);
								if(unary_rule)
								{
									double ruleprob=unary_rule->prob;
									if(ruleprob>0)
									{
										double beta=this->Get_inside_prob(index,rhsl);
										double rule_expect_count=alpha*ruleprob*beta/sentence_prob;
									    //unary_rule->expect_count+=rule_expect_count;
                                        //modify for multi thread training
                                        this->m_train_param.system_rules_expect_count[unary_rule->rule_id]+=rule_expect_count;
                                
									}
								}

								//binary rule
								for(int rhsr=0;rhsr<this->grammar.nonterminal_count;rhsr++)
								{
									
									Rule* same_rule=this->grammar.GetRule(lhs,rhsl,rhsr,bi_same);
									Rule* reverse_rule=this->grammar.GetRule(lhs,rhsl,rhsr,bi_reverse);
									double same_ruleprob=0.0;
									double reverse_ruleprob=0.0;

									if(same_rule)
										same_ruleprob=same_rule->prob;
									if(reverse_rule)
										reverse_ruleprob=reverse_rule->prob;
									if(same_ruleprob<=0.0 && reverse_ruleprob<=0.0)
									{
										if(this->debug_level>=3)
										{
											fprintf(Loger::logfile, "not found: %s,%s,%s\n", Symbols::invert_symbols[lhs].c_str(),Symbols::invert_symbols[rhsl].c_str(),Symbols::invert_symbols[rhsr].c_str());
											fprintf(Loger::logfile, "not found: %d,%d,%d\n", lhs,rhsl,rhsr);
										}
										continue;
									}
									

									double sbetasum=0; 
									double ibetasum=0;
									for(int esplit=estart;esplit<=eend;esplit++)
									{
										for(int csplit=cstart;csplit<=cend;csplit++)
										{
											double beta1=this->Get_inside_prob(estart,esplit,cstart,csplit,rhsl);
											double beta2=this->Get_inside_prob(esplit,eend,csplit,cend,rhsr);

											double beta3=this->Get_inside_prob(estart,esplit,csplit,cend,rhsl);
											double beta4=this->Get_inside_prob(esplit,eend,cstart,csplit,rhsr);

											sbetasum+=beta1*beta2;
											ibetasum+=beta3*beta4;
										}
									}//endof split
									double same_expect_count=alpha*same_ruleprob*sbetasum/sentence_prob;
									double reverse_expect_count=alpha*reverse_ruleprob*ibetasum/sentence_prob;

									if(same_rule)
									{
										//same_rule->expect_count+=same_expect_count;
                                        this->m_train_param.system_rules_expect_count[same_rule->rule_id]+=same_expect_count;
									}
									if(reverse_rule)
									{
										//reverse_rule->expect_count+=reverse_expect_count;
                                        this->m_train_param.system_rules_expect_count[reverse_rule->rule_id]+=reverse_expect_count;
									}
									if (this->debug_level>=3) 
									{
										fprintf(Loger::logfile, "- span = %d,%d,%d,%d\n", estart, eend,cstart, cend );
										fprintf(Loger::logfile, "** alpha = %g\n", alpha);
										fprintf(Loger::logfile, "** sBetasum = %g, sPr = %g\n", sbetasum, same_ruleprob);
										fprintf(Loger::logfile, "** iBetasum = %g, iPr = %g\n", ibetasum, reverse_ruleprob);
										fprintf(Loger::logfile, "** pi = %g\n", sentence_prob);
									 }
								}
							
							}

						}
					}
					
				}//endof eend
			}//endof estart
		}//endof cend
	}//endof cstart
}


/**
 * @brief Add probability to a cell.
 *
 * @param s estart  
 * @param t eend
 * @param u cstart 
 * @param v cend
 * @param lhs left hand side of the rule.
 * @param prob the inside probability 
 */
void Trainer::Increase_inside(int s,int t,int u,int v,int lhs,double prob)
{
	int index=this->GetArrayIndex(s,t,u,v);
	this->Increase_inside(index,lhs,prob);
}

void Trainer::Increase_inside(int index,int lhs,double prob)
{
	TableCell* cell=&ptable[index];
	cell->in_prob_dict[lhs]+=prob;
}

void Trainer::Increase_outside(int s,int t,int u,int v,int lhs,double prob)
{
	int index=this->GetArrayIndex(s,t,u,v);
	this->Increase_outside(index,lhs,prob);
}

void Trainer::Increase_outside(int index,int lhs,double prob)
{
	TableCell* cell=&ptable[index];
	cell->out_prob_dict[lhs]+=prob;
}

double Trainer::Get_inside_prob(int s,int t,int u,int v,int lhs)
{
	int index=this->GetArrayIndex(s,t,u,v);
	return this->Get_inside_prob(index,lhs);
}

double Trainer::Get_inside_prob(int index,int lhs)
{
	TableCell* cell=&ptable[index];
	return cell->in_prob_dict[lhs];
}

double Trainer::Get_outside_prob(int index, int lhs)
{
	TableCell* cell=&ptable[index];
	return cell->out_prob_dict[lhs];
}

double Trainer::Get_outside_prob(int s,int t,int u,int v, int lhs)
{
	int index=this->GetArrayIndex(s,t,u,v);
	return this->Get_outside_prob(index,lhs);
}



/**
 * @brief Use the open source implement of OWL-QN to optimize the feature weights.
 *
 * @param weights  feature weights
 */
void MStep_Trainer::OptimizeFeatureWeights_jp(vector<double>& weights)
{

    //if expect_count==0 L1 norm LBFGS error!!
    for(int i=0;i<this->m_scfg.phrase_rules_count;i++)
    {
        if(this->m_scfg.phrase_rules[i]->expect_count==0)
        {
            this->m_scfg.phrase_rules[i]->expect_count=1e-20;
            this->m_scfg.N_cnt[3]+=1e-20;
        }
    }

    cout<<"begin optimize the lambda!"<<endl;
    int N = FeatureManager::GetSingleton().GetFeatureCount();
    cout<<"the number of features is: "<<N<<endl;
    int  ret = 0;
    lbfgsfloatval_t fx=0;
    lbfgsfloatval_t *x = lbfgs_malloc(N);
    lbfgs_parameter_t param;

    if (x == NULL) {
        cout<<"ERROR: Failed to allocate a memory block for variables"<<endl;
    }

    /* Initialize the variables. */
    if(weights.size()>0)
    {
        for(int i=0;i<N;i++)
        {
            x[i]= weights[i];
        }
    }

    /* Initialize the parameters for the L-BFGS optimization. */
    lbfgs_parameter_init(&param);
    /*param.linesearch = LBFGS_LINESEARCH_BACKTRACKING;*/

    /*
       Start the L-BFGS optimization; this will invoke the callback functions
       evaluate() and progress() when necessary.
       */

    //we pre compute some value that used in gradient function 
    LBFGS_Param* lbfgs_param=new LBFGS_Param();
    lbfgs_param->scfg=&this->m_scfg;
    //int thread_count = this->config->configfile->read<int>("thread_count");
    double L2_coefficient=this->m_pconfig->configfile->read<double>("L2_coefficient");

    lbfgs_param->L2_coefficient= L2_coefficient;
    PreComputeLBFGS_Param(lbfgs_param,N);

    int max_iteration = this->m_pconfig->configfile->read<int>("max_iteration");
    int past=this->m_pconfig->configfile->read<int>("past");
    double delta=this->m_pconfig->configfile->read<double>("delta");
    int linesearch =this->m_pconfig->configfile->read<int>("linesearch");
    double gtol =this->m_pconfig->configfile->read<double>("gtol");
    double epsilon=this->m_pconfig->configfile->read<double>("epsilon");
    double L1_coefficient = this->m_pconfig->configfile->read<double>("L1_coefficient");
    int orthantwise_start = this->m_pconfig->configfile->read<int>("orthantwise_start");

    param.orthantwise_start = orthantwise_start;
    param.orthantwise_end = N-1;
    param.orthantwise_c=L1_coefficient;
    param.linesearch = linesearch;
    param.max_linesearch = 20;
    param.max_iterations =max_iteration;
    param.past=past;
    param.delta=delta;
    param.epsilon=epsilon;
    param.gtol=gtol;

    ret = lbfgs(N, x, &fx, evaluate_main, progress, static_cast<void*>(lbfgs_param), &param);

    Loger::LogTime();
    Loger::mylogfile<<"L-BFGS optimization terminated with status code = "<<ret<<endl;
    Loger::mylogfile<<"L-BFGS optimization log value fx = "<<fx<<endl;
    cout<<"L-BFGS optimization terminated with status code = "<<ret<<endl;
    cout<<"L-BFGS optimization log value fx = "<<fx<<endl;


    weights.clear();
    weights.resize(N,0);

    for(int i=0;i<N;i++)
    {
        weights[i]=x[i];
    }
    lbfgs_free(x);
}


/**
 * @brief  write the optimized weights to files
 *
 * @param weights  feature weights
 * @param scfg     grammar
 */
void MStep_Trainer::WriteFeature_Value_Weights(const vector<double>& weights,const SCFG& scfg)
{

    //we also write the terminal rule's feature weight into a file
    Features& feature = Features::GetSingleton();
    string fea_weight_filefullname=Tools::GetFileFullName("fea_weight");
    string fea_value_filefullname=Tools::GetFileFullName("fea_value");
    ofstream fea_weight_file(fea_weight_filefullname);
    ofstream fea_value_file(fea_value_filefullname);
    if(!fea_weight_file)
    {
        cerr<<"open feature weight file error!"<<fea_weight_filefullname<<endl;
    }
    if(!fea_value_file)
    {
        cerr<<"open feature value file error!"<<fea_value_filefullname<<endl;
    }

    for(int i=0;i<scfg.phrase_rules_count;i++)
    {
        PhraseRule* rule = scfg.phrase_rules[i];
        vector<pair<long long ,double> > rule_features = feature.rules_features_index_values[rule->rule_id];
                
        for(unsigned int i=0;i< rule_features.size();i++)
        {
            long long feature_index = rule_features[i].first;
            fea_weight_file<<weights[feature_index]<<" ";
            fea_value_file<<rule_features[i].second<<" ";
        }
        fea_weight_file<<endl;
        fea_value_file<<endl;
    }

   fea_weight_file.close();
   fea_value_file.close();
}


/**
 * @brief Used for printing the process of training procedure to the stdout.
 *
 * @param row   the row position 
 * @param col   the column position
 */
void Trainer::DisplayTrainProcess(int row,int col)
{
    move(row,col);
    string s="*";
    waddstr(stdscr,s.c_str());
    refresh();
}
