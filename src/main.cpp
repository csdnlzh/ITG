#include <iostream>
#include <thread>
#include <curses.h>
#include "config.h"
#include "corpus.h"
#include "scfg.h"
#include "trainer.h"
#include "loger.h"
#include "symbols.h"
#include "parser.h"
#include "aligner.h"
#include "pruning.h"
#include "globals.h"
#include "extractor.h"
#include "alignment.h"
#include "align_features.h"
#include "features_manager.h"
#include "dict_feature.h"
#include "eval.h"

using namespace std;

FILE* logfile=fopen ("train_log.txt" , "w");

void InitScreen(string msg)
{
    initscr();
    clear();
    move(0,0);
    waddstr(stdscr,msg.c_str());
    move(1,0);
    string msg2="the training process of each threads is: ";
    waddstr(stdscr,msg2.c_str());
    refresh();
}

void eval()
{
    
    vector<Alignment> gold_alignment;
    vector<Alignment> predict_alignment;
    AlignmentBuilder align_builder;
    string gold_align_filename=ITG::Config::GetSingleton().configfile->read<string>("gold_align");
    align_builder.BuildAlignments(gold_alignment,gold_align_filename);
    string predict_align_filename=ITG::Config::GetSingleton().configfile->read<string>("predict_align");
    predict_align_filename= Tools::GetFileFullName(predict_align_filename);
    align_builder.BuildAlignments(predict_alignment,predict_align_filename);

    Eval eval;
    eval.Run(gold_alignment,predict_alignment);
}
void predict_alignment_fun(SCFG& scfg,Corpus& corpus)
{
	ConfigFile* config=ITG::Config::GetSingleton().configfile;
	if(scfg.phrase_rules_count <=0)
	{
		string grammarfilename=config->read<string>("gfile");
		scfg.ReadGramma(grammarfilename);
	}
	double threshold_prob= scfg.ComputeThresholdProb();
	cout<<"threshold_prob is : "<<threshold_prob<<endl;

	int pstart= config->read<int>("pstart");
	int pend=config->read<int>("pend");
    pend=  (pend==0 ? corpus.paircount : pend);
    cout<<"we will decode the following sentence: "<<pstart<<"-"<<pend<<endl;
    int thread_count=config->read<int>("thread_count");
    int senpair_per_thread=(pend-pstart)/thread_count;


    vector<std::thread*> threads;
    vector<vector<string>* > alignments_v;
    vector<Aligner*> aligners;
    for(int i=0;i<thread_count+1;i++)
    {
        int start=i*senpair_per_thread+pstart;
        int end = ((i+1)*senpair_per_thread+pstart) < pend ? ((i+1)*senpair_per_thread+pstart) : pend ;
        if(i== thread_count)
        {
            end=pend;
        }
        if (start==end)
            break;
        vector<string>* alignments = new vector<string>();
        alignments_v.push_back(alignments);

        cout<<i<<"th thread: "<<start<<" "<<end<<endl;
        Aligner* aligner =new Aligner(i,corpus,scfg,corpus.senpairs,start,end,*alignments);
        aligner->SetRuleProbThreshold(threshold_prob);
        aligners.push_back(aligner);

        std::thread* t = new std::thread(*aligner);
        threads.push_back(t);
    }

    for(unsigned int i=0;i<threads.size();i++)
    {
        threads[i]->join();
        delete threads[i];
    }
    
    for(unsigned int i=0;i<aligners.size();i++)
    {
        delete aligners[i];
    }

    string align_file_name = ITG::Config::GetSingleton().configfile->read<string>("predict_align");
    align_file_name = Tools::GetFileFullName(align_file_name);
    cout<<"write alignment file "<<align_file_name<<endl;
    ofstream align_file(align_file_name);
    if(!align_file)
    {
        cout<<"open file error:  "<<align_file_name<<endl;
        return ;
    }
    for(unsigned int i=0;i<alignments_v.size();i++)
    {
        for(unsigned int j=0;j<alignments_v[i]->size();j++)
        {
            align_file<< (*alignments_v[i])[j];
        }

        delete alignments_v[i];
    }
    align_file.close();
    cout<<"write ok!"<<endl;

}

void  train_fun(SCFG& scfg,Corpus& corpus)
{
    ConfigFile* config=ITG::Config::GetSingleton().configfile;
	
    int use_features= config->read<int>("use_features");
    int iteration=config->read<int>("iteration");
    int system_rules_coun=scfg.unary_rules_count+scfg.binary_rules_count;
    int thread_count = config->read<int>("thread_count");
    int senpair_start = config->read<int>("senpair_start");
    int senpair_end = config->read<int>("senpair_end");
    int senpair_per_thread = (senpair_end-senpair_start)/thread_count;
    vector<double> weights;
    MStep_Trainer mstep_trainer(scfg);

    if(use_features)
    {
          Features::GetSingleton().ComputeActiveFeatures(scfg);
         // mstep_trainer.InitFeatureWeights_RuleProb(weights,scfg);
    }
        
    for(int it=0;it<iteration;it++)
    {
        string msg="we are at the "+Tools::i2str(it)+"th iteration training state ";
        InitScreen(msg);
        vector<std::thread*> threads;
        vector<Train_Param*> train_params;
        vector<double> total_log_prob(thread_count+1,0);

        for(int i=0;i<thread_count+1;i++)
        {
            int start=i*senpair_per_thread+senpair_start;
            int end = ((i+1)*senpair_per_thread+senpair_start) < senpair_end ? ((i+1)*senpair_per_thread+senpair_start) : senpair_end ;
            if (start==end)
                break;
            Train_Param* train_param =new Train_Param(4,system_rules_coun,scfg.phrase_rules_count);
            train_params.push_back(train_param);
            Trainer* trainer =new Trainer(i,corpus.senpairs,scfg,start,end, *train_param,total_log_prob);
            std::thread* t = new std::thread(*trainer);
            threads.push_back(t);
        }
       
        for(unsigned int i=0;i<threads.size();i++)
        {
            threads[i]->join();
            delete threads[i];
        }
    
        double sum_total_log_prob=0;
        for(unsigned int i=0;i<total_log_prob.size();i++)
        {
            sum_total_log_prob+=total_log_prob[i];
        }
        
        msg="logprob= "+Tools::d2str(sum_total_log_prob);
        msg+= " ok ,we have done the "+Tools::i2str(it)+"th iteration train. we will update the scfg rule probability now .";
        move(5+thread_count,0);
        waddstr(stdscr,msg.c_str());
        refresh();
        //clear();
        endwin();

        scfg.Reduce_SCFG_Params(train_params);

        if(use_features)
        {

            mstep_trainer.OptimizeFeatureWeights_jp(weights);
            scfg.Update(weights);
            if(it == iteration-1)
            {
                mstep_trainer.WriteFeature_Value_Weights(weights,scfg);
            }
        }
        else
        {
            int vb = config->read<int>("vb");
            scfg.Update(vb);
        }
        for(unsigned int i=0;i<train_params.size();i++)
        {
            delete train_params[i];
        }

        if(it>=0)
        {
            predict_alignment_fun(scfg,corpus);
            Loger::mylogfile<<it<<" iteration alignment result"<<endl;
            eval();
        }
    }

}

void extract_fun()
{
	ConfigFile* config=ITG::Config::GetSingleton().configfile;
	string afilename=config->read<string>("extract_align");
	vector<Alignment> alignments;
	AlignmentBuilder align_builder;
	align_builder.BuildAlignments(alignments,afilename);
	Extractor extractor;
	extractor.Run(alignments);
}


void InitFeatureTemplate()
{
    int use_features=ITG::Config::GetSingleton().configfile->read<int>("use_features");
    if(use_features)
    {
        Features& feature = Features::GetSingleton();
        feature.AddFeatureTemplate(new BasicFeature());
        int use_dict_feature=ITG::Config::GetSingleton().configfile->read<int>("use_dict_feature");
        if(use_dict_feature)
        {
            feature.AddFeatureTemplate(new DictFeature());
        }

    }
}

int main(int argc ,char* argv[])
{
	ConfigFile* config= ITG::Config::GetSingleton().configfile ;

	for(int i=1;i<argc;i=i+2)
	{
		string key(argv[i]);
		string value(argv[i+1]);
		config->remove(key);
		config->add<string>(key,value);
		cout<<"reset "<<key<<" = "<<value<<endl;
	}
	    
    	string runmode=config->read<string>("runmode");
	Corpus& corpus = Corpus::GetSingleton();
	corpus.ReadFile();

	Symbols::InitSymbols();
    
	SCFG scfg(corpus);

	if(runmode=="t" || runmode=="tp")
	{
		InitFeatureTemplate();
		scfg.InitSystemRules();
		scfg.InitPhraseRules();
		train_fun(scfg,corpus);
		scfg.WriteGramma("grammar.txt");
		scfg.WriteTransTable("trans.txt");
	}

	if(runmode=="p" || runmode=="tp")
	{
		predict_alignment_fun(scfg,corpus);
	}

	Loger::mylogfile.close();
	fclose(Loger::logfile);

	return 0;
}
