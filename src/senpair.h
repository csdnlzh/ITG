#ifndef Pair_H

#define Pair_H

#include <vector>
#include "globals.h"

using namespace std;

class SenPair
{

public:
	SenPair(int id,vector<int> cwordlist,vector<int> ewordlist)
	{
		this->m_id=id;
		this->m_cwordlist=cwordlist;
		this->m_ewordlist=ewordlist;
	};

	void  GetCPhrase(int cstart,int cend,vector<int>& cphrase);
	void  GetEPhrase(int estart,int eend,vector<int>& ephrase);

	int GetClen()
	{
		return m_cwordlist.size();
	}

	int GetElen()
	{
		return m_ewordlist.size();
	}
    
    int GetID()
    {
        return m_id;
    }

	vector<int> m_cwordlist;
	vector<int> m_ewordlist;

private:
    int m_id;
	void Split(const vector<int>& words ,int s,int t,vector<int>& phrase);
};

#endif
