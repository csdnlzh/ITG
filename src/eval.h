#ifndef Eval_H
#define Eval_H
#include <vector>
#include "alignment.h"

class Eval
{
public:
    Eval()
    {
        m_predict_count=0;  
        m_sure_intersect_count=0; 
        m_possible_intersect_count=0; 
        m_possible_count=0; 
        m_sure_count=0;   
    }

    void  Run(const vector<Alignment>& gold,const vector<Alignment>& predict);
private:
  
    int m_predict_count;  // |A|
    int m_sure_intersect_count; //|A n S|
    int m_possible_intersect_count; //|A n P|
    int m_possible_count; //|P|
    int m_sure_count;     //|S|

};
#endif
