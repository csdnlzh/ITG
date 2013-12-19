#include "extractor.h"

void Extractor::Run(vector<Alignment>& alignments)
{
	cout<<"start extract phrase pair ."<<endl;
	ConfigFile* config=ITG::Config::GetSingleton().configfile;
	int pstart= config->read<int>("senpair_start");
	int pend=config->read<int>("senpair_end");
	pend=  (pend==0 ? corpus.paircount : pend);

	//one-to-many one direction
	for(int i=pstart;i<pend;i++)
	{
		this->ExtractMain(corpus.senpairs[i],alignments[i]);
        if(i%1000==0)
        {
		    cout<<i<<endl;
        }
	}
	int two_direction=config->read<int>("two_direction");
	if(two_direction)
	{
		//one-to-many another direction
		int cmaxlen=this->c_max_phrase_len;
		int cminlen=this->c_min_phrase_len;
		this->c_max_phrase_len = this->e_max_phrase_len;
		this->c_min_phrase_len = this->e_min_phrase_len;
		this->e_max_phrase_len = cmaxlen;
		this->e_min_phrase_len = cminlen;
		for(int i=pstart;i<pend;i++)
		{
			this->ExtractMain(corpus.senpairs[i],alignments[i]);
            if(i%1000==0)
            {
			    cout<<i<<endl;
            }
		}
	}
	this->phrase_grammar_file.close();
	cout<<"extract phrase pair ok ."<<endl;
}

bool Extractor::IsGoodPhrasePair(const PhrasePair& pp,SenPair* senpair)
{

    for(int i=pp.estart;i<=pp.eend;i++)
    {
        string eword=corpus.edict_reverse[senpair->m_ewordlist.at(i)];
        if (this->prior_knowledge->IsPunctuation(eword))
            return false;
    }

    for(int j=pp.cstart;j<=pp.cend;j++)
    {
        string cword=corpus.cdict_reverse[senpair->m_cwordlist.at(j)];
        if (this->prior_knowledge->IsPunctuation(cword))
            return false;
    }

    return true;
}
void Extractor::ExtractMain(SenPair* senpair,Alignment& alignment)
{
	vector<PhrasePair> phrasepairs;
	this->Extract(phrasepairs,senpair->GetElen(),senpair->GetClen(),alignment);
	double defaultprob=1e-10;
	for(unsigned int i=0;i<phrasepairs.size();i++)
	{

		PhrasePair pp=phrasepairs[i];
        bool check_result=this->IsGoodPhrasePair(pp,senpair);
        if(!check_result)
        {
           // cout<<"no pass"<<endl;
            continue;
        }
        this->phrase_grammar_file<<defaultprob<<" \t";
		this->phrase_grammar_file<<"C -> ";

   
		// C -> ephrase | cphrase
		
		for(int i=pp.estart;i<=pp.eend;i++)
		{
			this->phrase_file<<corpus.edict_reverse[senpair->m_ewordlist.at(i)]<<" ";
			this->phrase_grammar_file<<senpair->m_ewordlist.at(i)<<" ";
		}

        this->phrase_file<<" | ";
		this->phrase_grammar_file<<" | ";
		
        for(int j=pp.cstart;j<=pp.cend;j++)
		{
			this->phrase_file<<corpus.cdict_reverse[senpair->m_cwordlist.at(j)]<<" ";
			this->phrase_grammar_file<<senpair->m_cwordlist.at(j)<<" ";
		}
	
		this->phrase_file<<endl;
		this->phrase_grammar_file<<endl;
	}
}
void Extractor::AddPhrasePairs(vector<PhrasePair>& phrasepairs, Alignment& alignment,int estart,int eend,int cstart,int cend,int clen)
{
	if(cend==-1) return;
	
	for(unsigned int i=0;i<alignment.links.size();i++)
	{
		if( alignment.links[i].first >=cstart && alignment.links[i].first<=cend )
		{
			if( alignment.links[i].second < estart || alignment.links[i].second > eend)
			{
				return ;
			}
		}
	}
	int cs=cstart;
	do 
	{
		int ce=cend;
		do
		{
			//the phrase length constraint , we can get one-to-many rules with this constraint
			if(eend-estart+1>=e_min_phrase_len && ce-cs+1>=c_min_phrase_len)
			{
				if(ce-cs+1<=c_max_phrase_len)
				{
					// the code of this scope is very sensitive ,important 
					int c_e_linkcount=0; 
					int e_c_linkcount=0;  

					// count how many links emit from the ch side
					for(int cindex=cs;cindex<=ce;cindex++)
					{
						if(c_align_flag[cindex])
							c_e_linkcount++;
					}
					// count how many links emit from the en side
					for(int eindex=estart;eindex<=eend;eindex++)
					{
						if(e_align_flag[eindex])
							e_c_linkcount++;
					}

					//max_phrase_link is a user define configuration variable , used for getting the non-compositional phrase
					if(c_e_linkcount<=max_phrase_link || e_c_linkcount<=max_phrase_link )
					{
						PhrasePair pp(estart,eend,cs,ce);
						phrasepairs.push_back(pp);
						ce++;
					}
					else
					{
						break;
					}
				}
				else
				{
					break;
				}
			}
			else
			{
				ce++;
			}

		}while( ce<clen&& (!c_align_flag[ce]));
		cs--;
	}while( cs>=0 && (!c_align_flag[cs]));
}
void Extractor::Extract(vector<PhrasePair>&phrasepairs ,int elen,int clen, Alignment& alignment)
{
	this->c_align_flag.clear();
	this->e_align_flag.clear();
	this->c_align_flag.resize(clen,0);
	this->e_align_flag.resize(elen,0);

	for(unsigned int i=0;i<alignment.links.size();i++)
	{
		c_align_flag[alignment.links[i].first]=1;
		e_align_flag[alignment.links[i].second]=1;
	}

	for(int estart=0;estart<elen;estart++)
	{
		int max_eend= min(estart+this->e_max_phrase_len,elen);
		for(int eend=estart;eend<max_eend;eend++)
		{
			int cstart=clen-1;
			int cend=-1;

			for(unsigned int i=0;i<alignment.links.size();i++)
			{
				if( alignment.links[i].second >=estart && alignment.links[i].second<=eend )
				{
					cstart=min(alignment.links[i].first,cstart);
					cend=max(alignment.links[i].first,cend);
				}
			}
			this->AddPhrasePairs(phrasepairs,alignment,estart,eend,cstart,cend,clen);
		}
	}
}
