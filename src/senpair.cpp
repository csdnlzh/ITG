#include "senpair.h"
#include <stdlib.h>

void  SenPair::GetCPhrase(int cstart,int cend,vector<int>& cphrase)
{
	return this->Split(this->m_cwordlist,cstart,cend,cphrase);
}
void  SenPair::GetEPhrase(int estart,int eend,vector<int>& ephrase)
{
	return this->Split(this->m_ewordlist,estart,eend,ephrase);
}

void  SenPair::Split(const vector<int>& words,int s,int t,vector<int>& phrase)
{

	if(t==s)
	{
		phrase.push_back(1);  // we use 1 indicate the NULL word
	}
	else
	{
		for (int i = s;i<t; i++)
		{
			phrase.push_back(words[i]);
		}
	}   
}
