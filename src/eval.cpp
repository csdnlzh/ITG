#include "eval.h"
#include "loger.h"
#include <iostream>

using namespace std;

void  Eval::Run(const vector<Alignment>& gold,const vector<Alignment>& predict)
{
    if(gold.size()!=predict.size())
    {
        cout<<"gold and predict alignment size isn't equal "<<endl;
        return ;
    }

    for(unsigned int i=0;i<gold.size();i++)
    {
        const Alignment& gold_align = gold[i];
        const Alignment& predict_align=predict[i];
        this->m_predict_count+=predict_align.links.size();
        this->m_possible_count+= gold_align.possible_links.size();
        this->m_sure_count+= gold_align.sure_links.size();


        for(unsigned int j=0;j<gold_align.sure_links.size();j++)
        {
            if(predict_align.IsHasLink(gold_align.sure_links[j]))
            {
                this->m_sure_intersect_count++;
            }
        }

        for(unsigned int j=0;j<gold_align.possible_links.size();j++)
        {
            if(predict_align.IsHasLink(gold_align.possible_links[j]))
            {
                this->m_possible_intersect_count++;
            }
        }
    }


    cout<<"predict              links count:\t"<<this->m_predict_count<<endl;
    cout<<"sure                 links count:\t"<<this->m_sure_count<<endl;
    cout<<"possible             links count:\t"<<this->m_possible_count<<endl;
    cout<<"sure intersect       links count:\t"<<this->m_sure_intersect_count<<endl;
    cout<<"possible intersect   links count:\t"<<this->m_possible_intersect_count<<endl;

    double precision = (double)this->m_possible_intersect_count/this->m_predict_count;
    double recall = (double) this->m_sure_intersect_count/this->m_sure_count;
  
    double aer_score =  1-(this->m_sure_intersect_count + this->m_possible_intersect_count)/(double)(this->m_sure_count+this->m_predict_count);

    cout<<"precision:\t"<<precision*100<<endl;
    cout<<"recall:\t"<<recall*100<<endl;
    cout<<"aer_score:\t"<<aer_score*100<<endl;
    
    Loger::LogTime();
    Loger::mylogfile<<"precision:\t"<<precision*100<<endl;
    Loger::mylogfile<<"recall:\t"<<recall*100<<endl;
    Loger::mylogfile<<"aer_score:\t"<<aer_score*100<<endl;

}
