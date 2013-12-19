#include <boost/algorithm/string.hpp>
#include <cmath>
#include <iostream>
#include "prior_knowledge.h"
#include "config.h"
#include "corpus.h"
using namespace std;

Prior_Knowledge* Prior_Knowledge::singleton= NULL;

void Prior_Knowledge::ReadLinks()
{
   	string linkfilename=ITG::Config::GetSingleton().configfile->read<string>("align_link");
	int max_len=ITG::Config::GetSingleton().configfile->read<int>("max_len");
	ifstream linkfile(linkfilename.c_str());
	if(!linkfile)
	{
		cerr<<"open alignment link file "<<linkfilename<<" error "<<endl;
		return ;
	}

	string line;
	int index=0;
	while(!linkfile.eof())
	{
		vector<vector<int > > sen_links;
		vector<int> c_e ;  //set -1 indicate there does't exit a alignment link
		c_e.resize(max_len,-1);
		vector<int> e_c ;
		e_c.resize(max_len,-1);
		
		getline(linkfile,line);
        boost::trim(line);
		if(line.length()>0)
		{
			stringstream links(line);
			int cpos;
			char con;
			int epos;
			while(!links.eof())
			{
				links>>cpos;
				links>>con;
				links>>epos;			
				c_e[cpos]=epos;
				e_c[epos]=cpos;
			}
		}
		sen_links.push_back(c_e);
		sen_links.push_back(e_c);
		this->intersection_links.push_back(sen_links);
		index++;
	}

}
Prior_Knowledge::Prior_Knowledge()
{
    corpus= &Corpus::GetSingleton();
    string runmode = ITG::Config::GetSingleton().configfile->read<string>("runmode");
    this->debug_level = ITG::Config::GetSingleton().configfile->read<int>("debug_level");
    int pruning = ITG::Config::GetSingleton().configfile->read<int>("pruning");
    if(pruning)
    {
        this->ReadLinks();
    }
    if(runmode=="t")
    {
        int use_pos_feature =ITG::Config::GetSingleton().configfile->read<int>("use_pos_feature");
        if(use_pos_feature )
        {
            this->ReadTagVocabulary();
            this->ReadTagProb();
            this->ReadSentenceTags();
        }
        int init_em_parameters=ITG::Config::GetSingleton().configfile->read<int>("init_em_parameters");
        if(init_em_parameters)
        {
            this->ReadInitProb();
        }

        int use_dict_feature=ITG::Config::GetSingleton().configfile->read<int>("use_dict_feature");
        if(use_dict_feature)
        {
            this->ReadDictfile();
        }

    }
    this->ReadPunctuation();
}

vector<vector<vector<int> > >& Prior_Knowledge::Model4_Intersection()
{
	return this->intersection_links;
}
void Prior_Knowledge::ReadInitProb()
{
    string c2efilename=ITG::Config::GetSingleton().configfile->read<string>("init_c2e_prob");
    string e2cfilename=ITG::Config::GetSingleton().configfile->read<string>("init_e2c_prob");
    this->FillTranTable(c2efilename,c2e_table_forInit);
    this->FillTranTable(e2cfilename,e2c_table_forInit);
}
void Prior_Knowledge::ReadTagProb()
{
    string c2e_tag_filename=ITG::Config::GetSingleton().configfile->read<string>("c2e_tag_prob");
    string e2c_tag_filename=ITG::Config::GetSingleton().configfile->read<string>("e2c_tag_prob");
    this->FillTranTable(c2e_tag_filename,c2e_tagtable);
    this->FillTranTable(e2c_tag_filename,e2c_tagtable);
}

void Prior_Knowledge::FillTranTable(const string& fileName,Tran_table& table)
{
    ifstream init_prob_file(fileName);
    if(!init_prob_file)
    {
        cout<<"open init prob file failed! "<<init_prob_file<<endl;
    }
    cout<<"begin read file: "<< fileName<<endl;
    string line;
    while(!init_prob_file.eof())
    { 
        getline(init_prob_file,line);

        boost::trim(line);
        if(line.length()==0)
        {
            continue;
        }
        stringstream linestream(line);
        int srcIndex;
        int trgIndex;
        double prob;
        linestream>>srcIndex;
        linestream>>trgIndex;
        linestream>>prob;
        table[srcIndex][trgIndex]=prob;
    } 
    cout<<"read file ok"<<endl;
}

// get p(item2|item1)
double Prior_Knowledge::GetItem2ItemProb(int item1,int item2,const Tran_table& trans_table)
{
    Tran_table::const_iterator item1_it;
    Tran_entry::const_iterator item2_it;
    item1_it = trans_table.find(item1);
    if(item1_it!= trans_table.end())
    {
        
        item2_it = item1_it->second.find(item2);
        if(item2_it!=item1_it->second.end())
        {
             return item2_it->second;
        }
    }
    return 1e-10;
}

double Prior_Knowledge::GetPhrase2PhraseProb(const vector<WORDTYPE>& phrase1,const vector<WORDTYPE>& phrase2,const Tran_table& tran_table)
{
    unsigned int phrase1_length= phrase1.size();
    double phrase12phrase2_prob=1.0;
    for(unsigned int i=0;i<phrase2.size();i++)
    {
        int word2=phrase2[i];
        double sum_phrase12word2=0;
        for(unsigned int j=0;j<phrase1.size();j++)
        {
            int word1=phrase1[j];
            double word12word2prob=this->GetItem2ItemProb(word1,word2,tran_table);
            sum_phrase12word2+=word12word2prob;
        }

        phrase12phrase2_prob=phrase12phrase2_prob*sum_phrase12word2;
    }

    phrase12phrase2_prob = phrase12phrase2_prob/phrase1_length;
    return phrase12phrase2_prob;
}

double Prior_Knowledge::GetRuleInitProb(const Rule& rule)
{
     PhraseRule* phrase = (PhraseRule*)&rule;
     //let's compute the translation probability of   e1e2..|c1c2c3.. 
     double cphrase2ephrase = this->GetPhrase2PhraseProb(phrase->m_cphrase,phrase->m_ephrase,this->c2e_table_forInit);
     //let's compute the translation probability of c1c2c3..|e1e2.. 
     double ephrase2cphrase = this->GetPhrase2PhraseProb(phrase->m_ephrase,phrase->m_cphrase,this->e2c_table_forInit);
//     Corpus& corpus = Corpus::GetSingleton();
//     cout<<*(phrase->ephrase)<<" "<< corpus.edict_reverse[*(phrase->ephrase)]<< " ";
//     cout<<*(phrase->cphrase)<<" "<< corpus.cdict_reverse[*(phrase->cphrase)]<<" ";
//     cout<< "e|c " << cphrase2ephrase<<endl;
     double init_prob=sqrt(cphrase2ephrase*ephrase2cphrase);
     return init_prob;
}


void Prior_Knowledge::DisplayRuleTags(const Rule& rule)
{
    PhraseRule* phrase_rule =(PhraseRule*)&rule;
    vector<std::unordered_map<int,int> > cwords_tags= phrase_rule->cwords_tags;
    vector<std::unordered_map<int,int> > ewords_tags= phrase_rule->ewords_tags;
   
    for(unsigned int i=0;i<ewords_tags.size();i++)
    {
        int eword_index=phrase_rule->m_ephrase[i];
        if(eword_index>1)
        {
            cout<<corpus->edict_reverse[eword_index]<<" ";
        }
        std::unordered_map<int,int>::iterator it= ewords_tags[i].begin();
        for(;it!=ewords_tags[i].end();it++)
        {
            cout<<this->epos_map_reverse[it->first]<<" "<<it->second<<" ";
        }
    }
    cout<<" | ";

    for(unsigned int i=0;i<cwords_tags.size();i++)
    {
        int cword_index= phrase_rule->m_cphrase[i];
        if(cword_index>1)
        {
            cout<<corpus->cdict_reverse[cword_index]<<" ";
        }
        std::unordered_map<int,int>::iterator it= cwords_tags[i].begin();
        for(;it!=cwords_tags[i].end();it++)
        {
            cout<<this->cpos_map_reverse[it->first]<<" "<<it->second<<" ";
        }
    }
    
    cout<<endl;
}

double Prior_Knowledge::GetRuleTagsScore(const Rule& rule)
{
    PhraseRule* phrase_rule=(PhraseRule*)&rule;
    cout<<phrase_rule->StrFormat()<<endl;
    if(this->debug_level>=2)
    {
        this->DisplayRuleTags(rule);
    }

    vector<std::unordered_map<int,int> > cwords_tags=phrase_rule->cwords_tags;
    vector<std::unordered_map<int,int> > ewords_tags=phrase_rule->ewords_tags;
    double score=0;

    //NULL word ,has no tag ,they can align any other word
    if(cwords_tags.size()==0 || ewords_tags.size()==0)
    {
        score=0;
    }

    vector<int> cwords_most_tags;
    vector<int> ewords_most_tags;

    //we get the most often tag for each cword
    for(size_t i=0;i<cwords_tags.size();i++)
    {
        std::unordered_map<int,int>::iterator it = cwords_tags[i].begin();
        int max_count=0;
        int most_often_tag= -1;
        for(;it!=cwords_tags[i].end();it++)
        {
            if(it->second>max_count)
            {
                max_count=it->second;
                most_often_tag=it->first;
            }
        }
        if(most_often_tag>=0)
        {
            cwords_most_tags.push_back(most_often_tag);
        }
    }

    //we get the most often tag for each eword
    for(size_t i=0;i<ewords_tags.size();i++)
    {
        std::unordered_map<int,int>::iterator it = ewords_tags[i].begin();
        int max_count=0;
        int most_often_tag= -1;
        for(;it!=ewords_tags[i].end();it++)
        {
            if(it->second>max_count)
            {
                max_count=it->second;
                most_often_tag=it->first;
            }
        }
        
        if(most_often_tag>=0)
        {
            ewords_most_tags.push_back(most_often_tag);
        }
    }
    for(size_t i=0;i<cwords_most_tags.size();i++)
    {
        for(size_t j=0;j<ewords_most_tags.size();j++)
        {
            if (cwords_most_tags[i]==ewords_most_tags[j])
            {
                score++;
            }
        }
    }

    if(this->debug_level>=2)
    {
        if (ewords_most_tags.size()>0)
        {
            cout<<pos_map_reverse[ewords_most_tags[0]];
        }
        if(cwords_most_tags.size()>0)
        {
           cout <<pos_map_reverse[cwords_most_tags[0]]<<endl;
        }
        cout<<"tag match score: "<<score<<endl;
        cout<<"-------------------------------------------------------------"<<endl;
    }
    return score;
}
double Prior_Knowledge::GetRuleTagsProb(const Rule& rule)
{
    PhraseRule* phrase_rule =(PhraseRule*)&rule;
    double ctags2etags=this->GetTags2TagsProb_forWords(phrase_rule->cwords_tags,phrase_rule->ewords_tags,c2e_tagtable);
    double etags2ctags=this->GetTags2TagsProb_forWords(phrase_rule->ewords_tags,phrase_rule->cwords_tags,e2c_tagtable);
    double score = sqrt(ctags2etags*etags2ctags);

    if(this->debug_level>=2)
    {
        cout<<phrase_rule->StrFormat()<<endl;
        this->DisplayRuleTags(rule);
        cout<<"tag match score: "<<score<<endl;
    }
    return score;
}
//compute the multi words tags tran probability p(trg_tags|src_tags)
double Prior_Knowledge::GetTags2TagsProb_forWords(const vector<std::unordered_map<int,int> >& src_tags,const vector<std::unordered_map<int,int> >& trg_tags,const Tran_table& tag_table)
{
    unsigned int trg_count= trg_tags.size();
    unsigned int src_count= src_tags.size();
    double total_prob=1.0;

    for(unsigned int i=0;i<trg_count;i++)
    {
        double sum_prob=0;
        for(unsigned int j=0;j<src_count;j++)
        {
            double src2trg_tags_prob=this->GetTags2TagsProb_forWord(src_tags[j],trg_tags[i],tag_table);
            sum_prob+=src2trg_tags_prob;
        }
        sum_prob=sum_prob/src_count;
        total_prob=total_prob*sum_prob;
    }
    return total_prob;
}

//compute a src word's tags to a trg word's tags prob; because a word may have several tags ,we used unordered_map store the tag id
//and the occur times of the tag, when compute the prob ,we use the fraction prob for normalization reasion  
double Prior_Knowledge::GetTags2TagsProb_forWord(const std::unordered_map<int,int>& src_tags,const std::unordered_map<int,int>& trg_tags,const Tran_table& tag_table)
{
    std::unordered_map<int,int>::const_iterator src_it,trg_it;
    double src_tags_total_count=0;
    double trg_tags_total_count=0;
    for(src_it=src_tags.begin();src_it!=src_tags.end();src_it++)
    {
        src_tags_total_count+=src_it->second;
    }

    for(trg_it=trg_tags.begin();trg_it!=trg_tags.end();trg_it++)
    {
        trg_tags_total_count+=trg_it->second;
    }
    double total_prob=0;
    for(trg_it=trg_tags.begin();trg_it!=trg_tags.end();trg_it++)
    {
        int trg_tag=trg_it->first;
        double sum_prob=0;
        for(src_it=src_tags.begin();src_it!=src_tags.end();src_it++)
        {
            int src_tag=src_it->first;
            double tag2tag_prob=this->GetItem2ItemProb(src_tag,trg_tag,tag_table);
            double fraction_prob=tag2tag_prob*(src_it->second/src_tags_total_count);
            sum_prob+=fraction_prob;
        }
        total_prob+=sum_prob*(trg_it->second/trg_tags_total_count);
    }
    return total_prob;
}
   
void Prior_Knowledge::ReadSentenceTags()
{
    string c_tags_filename=ITG::Config::GetSingleton().configfile->read<string>("c_tags");
    string e_tags_filename=ITG::Config::GetSingleton().configfile->read<string>("e_tags");
    ReadSentenceTagsFile(c_tags_filename,c_sentences_tags,cpos_map);
    ReadSentenceTagsFile(e_tags_filename,e_sentences_tags,epos_map);
}

void Prior_Knowledge::ReadSentenceTagsFile(const string& filename,vector<vector<int> >& sentences_tags,std::unordered_map<string,int>& dict)
{
    ifstream tagsfile(filename);
    if(!tagsfile)
    {
        cout<<"read tags file failed: "<< filename<<endl;
    }
    cout<<"start reading "<<filename<<" tags file"<<endl;
    string linestr;
    while(!tagsfile.eof())
    {
        getline(tagsfile,linestr);
        boost::trim(linestr);
        if(linestr.length()==0)
            continue;
        vector<int> tags;
        stringstream linestream(linestr);
        while(!linestream.eof())
        {
            int tag_id;
            string tag;
            linestream>>tag;
            tag_id=dict[tag];
            tags.push_back(tag_id);
        }
        sentences_tags.push_back(tags);
    }
    cout<<"read ok!"<<endl;
}
void Prior_Knowledge::GetCPhraseTags(int sentence_id,int start,int end,vector<int>& words_tags)
{
    words_tags.clear();
    for(int i=start;i<end;i++)
    {
        words_tags.push_back(this->c_sentences_tags[sentence_id][i]);
    }
}
void Prior_Knowledge::GetEPhraseTags(int sentence_id,int start,int end,vector<int>& words_tags)
{
    words_tags.clear();
    for(int i=start;i<end;i++)
    {
        words_tags.push_back(this->e_sentences_tags[sentence_id][i]);
    }
}

void Prior_Knowledge::ReadDictfile()
{
    string dictfile_name=ITG::Config::GetSingleton().configfile->read<string>("dictfile");
    ifstream dictfile(dictfile_name);
    if(!dictfile)
    {
        cerr<<"read dict file failed!  "<<dictfile_name<<endl;
    }
    string linestr;
    while(!dictfile.eof())
    {
        getline(dictfile,linestr);
        boost::trim(linestr);
        if(linestr.length()==0)
            continue;

        vector<string> c_e_phrase;
        boost::split(c_e_phrase,linestr,boost::algorithm::is_any_of("\t"));

        string cstring=c_e_phrase[0];
        string estrings=c_e_phrase[1];
      
        //cout<<"estrings: "<<estrings<<endl;
        vector<string> string_v;
        boost::split(string_v,estrings,boost::algorithm::is_any_of("/"));
        for(unsigned int i=0;i<string_v.size();i++)
        {
            if(string_v[i].length()>0)
            {
                this->dict[cstring][string_v[i]]=1;
               // cout<<"after split: "<<i<<" "<<string_v[i]<<endl;
            }
        }
    }
}
bool Prior_Knowledge::IsContainedInDict(const string& cphrase,const string& ephrase)
{
    Dict_table::iterator c_entry=dict.find(cphrase);
    if(c_entry!=dict.end())
    {
        Dict_entry::iterator e_entry=c_entry->second.find(ephrase);
        if(e_entry!=c_entry->second.end())
        {
            // cout<<"chinese phrase: "<<cphrase<<"  english phrase: "<<ephrase<<endl;
            return true;
        }
    }
   // cout<<" not in the dict "<<endl;
    return false;
}

void Prior_Knowledge::ReadTagVocabulary()
{
      string cvocfile_name=ITG::Config::GetSingleton().configfile->read<string>("c_pos_vocabulary");
      string evocfile_name=ITG::Config::GetSingleton().configfile->read<string>("e_pos_vocabulary");
      this->BuildTagVocabulary(cvocfile_name,this->cpos_map,this->cpos_map_reverse);
      this->BuildTagVocabulary(evocfile_name,this->epos_map,this->epos_map_reverse);
      
      //string pos_vocabulary_name=ITG::Config::GetSingleton().configfile->read<string>("pos_vocabulary");
      //this->BuildTagVocabulary(pos_vocabulary_name,this->pos_map,this->pos_map_reverse);
}

void Prior_Knowledge::BuildTagVocabulary(const string& filename,std::unordered_map<string,int>& dict, std::unordered_map<int,string>& dict_reverse)
{
    ifstream infile(filename);
    if(!infile)
    {
        cerr<<"open file failed! "<<filename<<endl;
    }

    string linestr;
    while(!infile.eof())
    {
        getline(infile,linestr);
        stringstream linestream(linestr);
        string tag;
        int index;
        linestream>>index;
        linestream>>tag;
        dict[tag]=index;
        dict_reverse[index]=tag;
    }
    infile.close();
}

void Prior_Knowledge::ReadPunctuation()
{
   // string punctuation_file=ITG::Config::GetSingleton().configfile->read<string>("punctuation");
   // ifstream infile(punctuation_file);
   // if(!infile)
   // {
   //     cerr<<"open file failed "<<punctuation_file<<endl;
   // }
   // 
   // string linestr;
   // while(!infile.eof())
   // {
   //     getline(infile,linestr);
   //     this->punctuation_map[linestr]=1;
   // }
   // infile.close();
}

bool Prior_Knowledge::IsPunctuation(string term)
{
    if (this->punctuation_map.find(term)!= this->punctuation_map.end())
        return true;
    else
        return false;
}
