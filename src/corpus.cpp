#include<string>
#include<iostream>
#include<vector>
#include<fstream>
#include<sstream>
#include<map>
#include<string>
#include"corpus.h"
#include"loger.h"
#include"tools.h"
#include "config.h"
using namespace std;
Corpus* Corpus::instance=NULL;

void Corpus::ReadFile()
{
	ifstream cfile(this->cfilename.c_str());
	ifstream efile(this->efilename.c_str());
	string cstr;
	string estr;
	int pairID=0;

	while(!efile.eof()&&!cfile.eof())
	{
		getline(cfile,cstr);
		getline(efile,estr);
		if (cstr=="" || estr=="")
		{
			continue;
		}
		int token;
		vector<int>  cwordv;
		vector<int>  ewordv;
		stringstream ctokens(cstr);
		
		while(ctokens>>token)
		{
			cwordv.push_back(token);
		}
		stringstream etokens(estr);
		while(etokens>>token)
		{
			ewordv.push_back(token);
		}

		SenPair * senpair=new SenPair(pairID,cwordv,ewordv);
		this->senpairs.push_back(senpair);
		pairID++;
	}
	this->paircount=pairID;
	cfile.close();
	efile.close();

	string cvcbfilename=ITG::Config::GetSingleton().configfile->read<string>("cvocab");
	string evcbfilename=ITG::Config::GetSingleton().configfile->read<string>("evocab");
	this->ReadVcb(cvcbfilename,this->cdict_reverse,this->cdict);
	this->ReadVcb(evcbfilename,this->edict_reverse,this->edict);
	this->cwordcount=this->cdict_reverse.size();
	this->ewordcount=this->edict_reverse.size();
	this->ShowStat();
}


void Corpus::ShowStat()
{
	cout<<"sentence count:"<<this->paircount<<endl;
	cout<<"cword count: "<<this->cwordcount<<endl;
	cout<<"eword count: "<<this->ewordcount<<endl;

	vector<string> msgs;
	msgs.push_back("sentence count: "+ Tools::i2str(this->paircount));
	msgs.push_back("chinese word count: "+ Tools::i2str(this->cwordcount));
	msgs.push_back("english word count: "+ Tools::i2str(this->ewordcount));
	Loger::log(msgs);
}

void Corpus::ReadVcb(string filename,std::unordered_map<int,string>& dict_reverse,std::unordered_map<string,int>& dict)
{
	ifstream vcbfile(filename.c_str());

    if(!vcbfile)
    {
        cout<<"read vcb file error ! "<<filename<<endl;
    }
	string line;
	while(!vcbfile.eof())
	{
		getline(vcbfile,line);
		stringstream tokens(line);
		int wordindex;
		tokens>>wordindex;
		string word;
		tokens>>word;

		dict_reverse[wordindex]=word;
		dict[word]=wordindex;
	}
	
	vcbfile.close();

}
