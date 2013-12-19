#ifndef Extractor_H
#define Extractor_H
#include<fstream>
#include "senpair.h"
#include "alignment.h"
#include "phrase_pair.h"
#include "corpus.h"
#include "tools.h"
#include "prior_knowledge.h"

using namespace std;
class Extractor
{
public:
	Extractor():corpus(Corpus::GetSingleton())
	{
		e_max_phrase_len=ITG::Config::GetSingleton().configfile->read<int>("e_max_phrase_len");
		e_min_phrase_len=ITG::Config::GetSingleton().configfile->read<int>("e_min_phrase_len");
		c_max_phrase_len=ITG::Config::GetSingleton().configfile->read<int>("c_max_phrase_len");
		c_min_phrase_len=ITG::Config::GetSingleton().configfile->read<int>("c_min_phrase_len");
		max_phrase_link=ITG::Config::GetSingleton().configfile->read<int>("max_phrase_link");
		phrase_file.open(Tools::GetFileFullName("phrase.txt"),ios::out);
		phrase_grammar_file.open(Tools::GetFileFullName("phrase_grammar.txt"),ios::out);

        prior_knowledge= &Prior_Knowledge::GetInstance();
	}

	~Extractor()
	{
		this->phrase_file.close();
		phrase_grammar_file.close();
	}

	//extract all the phrase pairs consistent with a word alignment
	//input is a word alignment A and sentence pair (e,f)
	//output is a set of phrase pairs
	void Extract(vector<PhrasePair>&phrasepairs, int elen,int clen, Alignment& alignment);

	void ExtractMain(SenPair* senpair,Alignment& alignment);

	void Run(vector<Alignment>& alignments);
    bool IsGoodPhrasePair(const PhrasePair& pp,SenPair* senpair);
private:

	void AddPhrasePairs(vector<PhrasePair>& phrasepairs,Alignment& alignment,int estart,int eend,int cstart,int cend,int clen);
	int e_max_phrase_len;
	int e_min_phrase_len;
	int c_max_phrase_len;
	int c_min_phrase_len;
	int max_phrase_link;
	
	vector<int> c_align_flag;
	vector<int> e_align_flag;
	ofstream phrase_file;
	ofstream phrase_grammar_file;
	Corpus& corpus ; 
    Prior_Knowledge*  prior_knowledge;

};
#endif
