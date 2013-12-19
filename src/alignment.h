#ifndef Alignment_H
#define Alignment_H
#include <vector>
#include <utility>
#include <sstream>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include "tools.h"

using namespace std;

class Alignment
{
public:
	Alignment(string& str)
	{
		boost::trim(str);
		if(str.length()==0) 
			return ;

		stringstream ss(str);

		while(!ss.eof())
		{
			int c;
			string str_e;
            int e;
			char cat;
			ss>>c;
			ss>>cat;
			ss>>str_e;

            int s_index=str_e.find('S');
            int p_index=str_e.find('P');

            
            if(s_index>=0)
            {
                 string substr=str_e.substr(0,s_index);
                 e=atoi(substr.c_str());
                 pair<int,int> link(c,e);
                 sure_links.push_back(link);
                 possible_links.push_back(link);
            }
             
            if(p_index>=0)
            {
                string substr=str_e.substr(0,p_index);
                e=atoi(substr.c_str());
                pair<int,int> link(c,e);
                possible_links.push_back(link);
            }

            if(s_index<0 && p_index<0)
            {
                e=atoi(str_e.c_str());
                pair<int,int> link(c,e);
                links.push_back(link);
            }
		}
	}


    //this method can only be called by predict alignment
    bool IsHasLink(const pair<int,int>& link) const
    {
        for(unsigned int i=0;i<links.size();i++)
        {
            if(links[i].first==link.first && links[i].second==link.second)
                return true;
        }
        return false;
    }
	vector<pair<int,int> > links;

    //use for evaluation
    vector<pair<int,int> > sure_links;
    vector<pair<int,int> > possible_links;
};

class AlignmentBuilder
{
public:
	AlignmentBuilder()
	{

	}
	void BuildAlignments(vector<Alignment>& alignments,string afilename);
};

#endif
