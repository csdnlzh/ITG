#ifndef Corpus_H

#define Corpus_H

#include "senpair.h"
#include "config.h"
#include <unordered_map>
#include<vector>
#include<string>

using namespace std;

class Corpus
{
public:
	
	static Corpus& GetSingleton()
	{
        if(!instance)
        {
            instance=new Corpus();
        }
		return *instance;
	}
	void ShowStat();
	void ReadFile();
	int paircount;
	std::unordered_map<int,string> cdict_reverse;
	std::unordered_map<int,string> edict_reverse;
	std::unordered_map<string,int> cdict;
	std::unordered_map<string,int> edict;
	vector<SenPair*> senpairs;
    
private:
	
    Corpus()
    {
        this->cfilename=ITG::Config::GetSingleton().configfile->read<string>("cfile");
        this->efilename=ITG::Config::GetSingleton().configfile->read<string>("efile");
        this->paircount=0;
        cwordcount=0;
        ewordcount=0;
    }
	
    static Corpus* instance;
	string cfilename;
	string efilename;
	int cwordcount;
	int ewordcount;
	void ReadVcb(string filename,std::unordered_map<int,string>& dict_reverse,std::unordered_map<string,int>& dict);
};



#endif
