#include <fstream>
#include "parser.h"


/**
 * @brief The entry function for parsing one bilingual sentence pair.
 *
 * @param senpair  The bilingual sentence pair to be parsed.
 *
 * @return  The Chart.
 */
ParseTableCell*  Parser::Parse(SenPair* senpair)
{
	this->senpair = senpair;
	this->elen=senpair->GetElen();
	this->clen=senpair->GetClen();
	this->e_dim=elen+1;
	this->c_dim=clen+1;
	this->e_dim_c_dim_c_dim= e_dim*c_dim*c_dim; // for efficient;
	this->c_dim_c_dim=c_dim*c_dim; // for efficient;
	this->InitIndicateTable();
	if(this->pruning)
	{
		this->Pruning();
	}
	this->InitTable();
	this->ChartParse();  //search the best tree
	this->ReleaseTable();
	return ptable;
}


/**
 * @brief The viterbi parser 
 */
void Parser::ChartParse()
{

	//the length of span from 1 ,because the base case has been formed in the inittable function
	for(int espan=1;espan<=this->elen;espan++)
	{
		for(int estart=0;estart<=this->elen-espan;estart++)
		{
			int eend=estart+espan;
			vector<WORDTYPE> ephrase;
            this->senpair->GetEPhrase(estart,eend,ephrase);

			for(int cspan=1;cspan<=this->clen;cspan++)
			{
				for(int cstart=0;cstart<=this->clen-cspan;cstart++)
				{
					int cend=cstart+cspan;
					int index=this->GetArrayIndex(estart,eend,cstart,cend);
					if(pruning && pindicate_table[index]>this->conflict_link)
					{
                        prune_count++;
						continue;
					}
                    vector<WORDTYPE> cphrase;
					this->senpair->GetCPhrase(cstart,cend,cphrase);
					Rule* rule=this->grammar.GetPhrase2PhraseRule(ephrase,cphrase);
					if(rule)  //if we can get a rule that can cover the span 
					{
						vector<int> children_index;
						this->Increase_inside(estart,eend,cstart,cend,rule->left,rule->prob,children_index,rule);
					}
					//else   //we search for a binary branch
					//{
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
									vector<int> children_index;
                                    children_index.reserve(8);
									double betal=0;
									double betar=0;
									if(rule->type==bi_reverse)
									{
										betal=this->Get_inside_prob(estart,esplit,csplit,cend,rhsl);
										if(betal<=0.0) continue;
										betar=this->Get_inside_prob(esplit,eend,cstart,csplit,rhsr);
										if(betar<=0.0) continue;

										children_index.push_back(estart);
										children_index.push_back(esplit);
										children_index.push_back(csplit);
										children_index.push_back(cend);
										children_index.push_back(esplit);
										children_index.push_back(eend);
										children_index.push_back(cstart);
										children_index.push_back(csplit);
									}
									else
									{
										betal=this->Get_inside_prob(estart,esplit,cstart,csplit,rhsl);
										if(betal<=0.0) continue;
										betar=this->Get_inside_prob(esplit,eend,csplit,cend,rhsr);
										if(betar<=0.0) continue;

										children_index.push_back(estart);
										children_index.push_back(esplit);
										children_index.push_back(cstart);
										children_index.push_back(csplit);
										children_index.push_back(esplit);
										children_index.push_back(eend);
										children_index.push_back(csplit);
										children_index.push_back(cend);
									}
									double mass=rule->prob*betal*betar;
									this->Increase_inside(estart,eend,cstart,cend,lhs,mass,children_index,rule);
								}
							}
						}
					//}//else
					if(espan==elen && cspan==clen)
					{
						//for every unary rules
						for(int unaryno=0;unaryno<this->grammar.unary_rules_count;unaryno++)
						{
							vector<int> children_index;
							Rule* rule=this->grammar.unary_rules[unaryno];
							int lhs=rule->left;
							int rhs=rule->right1;
						
							double inprob=this->Get_inside_prob(estart,eend,cstart,cend,rhs);
								
							if(inprob>0)
							{
								double mass=rule->prob*inprob;
								this->Increase_inside(estart,eend,cstart,cend,lhs,mass,children_index,rule);
							}
						}
					}	
				}//endof estart
			}//endof espan
		}//endof cstart
	}//endof cspan
}
void Parser::InitTable()
{
	int count=e_dim*e_dim*c_dim*c_dim;
	this->ptable = new ParseTableCell[count];

	//init the table with phrase translation probability
	for(int espan=0;espan<=1;espan++)
	{
		for(int estart=0;estart<=this->elen-espan;estart++)
		{
			int eend=estart+espan;
			
            vector<WORDTYPE> ephrase;
			//get the ch phrase according to the ch span index
			this->senpair->GetEPhrase(estart,eend,ephrase);

			for(int cspan=0;cspan<=1;cspan++)
			{
				if(espan==0 && cspan==0)
					continue;

				for(int cstart=0;cstart<=this->clen-cspan;cstart++)
				{
					int cend=cstart+cspan;

					//get the eng phrase according to the eng span index
                    vector<WORDTYPE> cphrase;
					this->senpair->GetCPhrase(cstart,cend,cphrase);

					//how to deal with if we can't find the proper rule ?
					Rule* rule = this->grammar.GetPhrase2PhraseRule(ephrase,cphrase);
					if(rule && rule->prob>0)
					{	
						vector<int> children_index;
						this->Increase_inside(estart,eend,cstart,cend,rule->left,rule->prob,children_index,rule);
					} 
				}
			}
		}
	}
}

void Parser::Increase_inside(int s,int t,int u,int v,int lhs,double prob,vector<int>& children_index,Rule* bestrule)
{
	int index=this->GetArrayIndex(s,t,u,v);
	ParseTableCell* cell=&ptable[index];

	if(cell->in_prob_dict[lhs]>=prob)
		return ;
	cell->in_prob_dict[lhs]=prob;
	cell->rule[lhs]=bestrule;
	cell->children_index[lhs]=children_index;

    //for debug
    if(lhs==3)
    {
        PhraseRule* phrase_rule=(PhraseRule*)bestrule;
        if (phrase_rule->m_ephrase.size()>1 || phrase_rule->m_cphrase.size()>1)
        {
            cout<<"in parser:\t"<<phrase_rule->StrFormat()<<endl;
            this->multi_words_rules_count++;
        }
    }
}

double Parser::Get_inside_prob(int s,int t,int u,int v,int lhs)
{
	int index=this->GetArrayIndex(s,t,u,v);
	ParseTableCell* cell=&ptable[index];
	return cell->in_prob_dict[lhs];
}



/**
 * @brief Initialize the indicate table for pruning.
 */
void Parser::InitIndicateTable()
{
	int count=e_dim*e_dim*c_dim*c_dim;
	this->pindicate_table= new int[count];
	memset(this->pindicate_table,0,sizeof(int)*count);
}



/**
 * @brief The pruning function for the viterbi parser. 
 * We use the fix-link pruning method.
 */
void Parser::Pruning()
{
	vector<int> c_e=Prior_Knowledge::GetInstance().Model4_Intersection()[this->senpair->GetID()][0];
	vector<int> e_c=Prior_Knowledge::GetInstance().Model4_Intersection()[this->senpair->GetID()][1];
	
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
					
				}
			}
		}
	}
}

void Parser::ReleaseTable()
{
	delete [] pindicate_table;
}
