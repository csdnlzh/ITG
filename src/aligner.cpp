#include "aligner.h"
#include <iostream>
#include <assert.h>
#include "parser.h"

using namespace std;

/**
 * @file aligner.cpp
 * @brief  Used for compute alignment for a bilingual sentence. In this class,
 * we first call the parser to parse the bilingual sentence, then we get the viterbi parse tree and collect the leaf nodes of the tree as the alignment
 * @author lizhonghua
 * @version 1.0
 * @date 2012-03-29
 */



/**
 * @brief  Entry point for multithreading.
 */
void Aligner::operator()()
{
    Parser parser(this->m_scfg);
    int count=0;
    for(int i=m_sen_start;i<m_sen_end;i++)
    {
        this->best_tree_rules.clear();
        this->senpair=m_senpairs[i];

        this->ptable= parser.Parse(senpair);
        this->elen=senpair->GetElen();
        this->clen=senpair->GetClen();

        this->e_dim=elen+1;
        this->c_dim=clen+1;
        this->e_dim_c_dim_c_dim= e_dim*c_dim*c_dim; // for efficient;
        this->c_dim_c_dim=c_dim*c_dim; // for efficient;

        int index=this->GetArrayIndex(0,elen,0,clen);
        ParseTableCell* cell=&ptable[index];
        if(cell)  //if parse right
        {
            this->CollectRules(cell,0);  // retrieve the rules from top to down in the best tree.
        }
        else
        {
            cout<<"parse error!"<<endl;
        }
    
        this->ReleaseTable();
        this->WriteAlignment();
        count++;
        if(count%10==0)
        {
            cout<<i<<endl;
        }
    }

    cout<<this->m_aligner_id<<"  pruned count: "<<parser.prune_count<<endl;
}




/**
 * @brief  collect leaf nodes of the viterbi parse tree.
 *
 * @param cell
 * @param lhs
 */
void Aligner::CollectRules(ParseTableCell* cell,int lhs)
{
	Rule* rule=cell->rule[lhs];
    if(!rule)
    {
        cout<<"parse error"<<endl;
        cout<<this->senpair->GetID()<<endl;
        return ;
    }

	
	if(rule->prob>=this->m_rule_prob_threshold)
	{
		this->best_tree_rules.push_back(rule); //collect this rule
	}

    //log the viterbi parsing tree
    //    this->tree_file<<"( "<<Symbols::invert_symbols[lhs]<<" ";
    //	
    //	if(rule->type == term)
    //	{
    //		this->tree_file<<" ( ";
    //		PhraseRule* phrase_rule = (PhraseRule* ) rule;
    //        for(unsigned int i=0;i<phrase_rule->m_ephrase.size();i++)
    //		{
    //            int ewordindex=phrase_rule->m_ephrase[i];
    //			this->tree_file<<this->m_corpus.edict_reverse[ewordindex]<<"-";
    //		}
    //		for(unsigned int i=0;i<phrase_rule->m_cphrase.size();i++)
    //		{
    //            int cwordindex=phrase_rule->m_cphrase[i];
    //			this->tree_file<<this->m_corpus.cdict_reverse[cwordindex]<<"-";
    //		}
    //        this->tree_file<<" ";
    //	 	this->tree_file<<phrase_rule->prob;
    //	  	this->tree_file<<" ) ";
    //	}

	if(rule->type== unary)
	{
		this->CollectRules(cell,rule->right1);
	}
	if(rule->type == bi_same || rule->type==bi_reverse)
	{
		vector<int> children_index=cell->children_index[lhs];
		int indexl=this->GetArrayIndex(children_index[0],children_index[1],children_index[2],children_index[3]);
		ParseTableCell* celll=&ptable[indexl];
		int indexr=this->GetArrayIndex(children_index[4],children_index[5],children_index[6],children_index[7]);
		ParseTableCell* cellr=&ptable[indexr];

		if(celll)
		{
			this->CollectRules(celll,rule->right1);
		}
		if(cellr)
		{
			this->CollectRules(cellr,rule->right2);
		}
	}

	//this->tree_file<<" ) ";
}
int Aligner::GetPosition(string lang,int word)
{
	vector<int> words;
	if(lang=="c")
	{
		words=senpair->m_cwordlist;
	}
	else
	{
		words=senpair->m_ewordlist;
	}
	for(unsigned int i=0;i<words.size();i++)
	{
		if(words[i]==word)
		{
			return i;
		}
	}
	return -1;
}



/**
 * @brief  Write the alignment to the file.
 */
void Aligner::WriteAlignment()
{
    ostringstream alignment;
	for(unsigned int i=0;i<this->best_tree_rules.size();i++)
	{
		Rule * rule = this->best_tree_rules[i];
		if(rule->type != term)
		{
			continue;
		}
		PhraseRule* ph_rule=(PhraseRule*)rule;

		vector<WORDTYPE> ephrase=ph_rule->m_ephrase;
		vector<WORDTYPE> cphrase=ph_rule->m_cphrase;
        unsigned int  ephrase_length=ephrase.size();
        unsigned int  cphrase_length=cphrase.size();
        if(ephrase_length>=2 || cphrase_length>=2)
        {
            this->multi_words_rules_count++;
            cout<<"in aligner:\t"<<ph_rule->StrFormat()<<endl;
        }

		vector<int> eword_positions;
		vector<int> cword_positions;
		vector<string> ewords;
		vector<string> cwords;

		for(unsigned int i=0;i<ephrase_length;i++)
		{
			int eword=ephrase[i];
			if(eword>1)
			{
				int eword_position=this->GetPosition("e",eword);
				eword_positions.push_back(eword_position);
			}
			string eword_human=this->m_corpus.edict_reverse[eword];
			ewords.push_back(eword_human);
		}
		
	    for(unsigned int i=0;i<cphrase_length;i++)
		{
			int cword=cphrase[i];
			if(cword>1)
			{
				int cword_position=this->GetPosition("c",cword);
				cword_positions.push_back(cword_position);
			}
			string cword_human=this->m_corpus.cdict_reverse[cword];
			cwords.push_back(cword_human);
		}

		if(eword_positions.size()>0 && cword_positions.size()>0)
		{
			for(unsigned int i=0;i<cword_positions.size();i++)
			{
				for(unsigned int j=0;j<eword_positions.size();j++)
				{
					alignment<<cword_positions[i];
					alignment<<"-";
					alignment<<eword_positions[j];
					alignment<<" ";

				//	this->alignment_human_file<<cwords[i];
				//	this->alignment_human_file<<"-";
				//	this->alignment_human_file<<ewords[j];
				//	this->alignment_human_file<<" ";
				}
			}
		}

		alignment<<"\t";
	//	this->alignment_human_file<<"\t";
	}

	alignment<<endl;
    this->m_alignments.push_back(alignment.str());
	//this->alignment_human_file<<endl;
}


/**
 * @brief release the cell table for the parse tree.
 */
void Aligner::ReleaseTable()
{
	delete [] ptable;
}
