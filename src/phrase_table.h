#ifndef Phrase_Table_H
#define Phrase_Table_H
#include <unordered_map>
#include <vector>
#include "globals.h"
using namespace std;

// definitions for phrase table
// deep compare of zero-terminated strings of ints
//struct eqphrase {
//  bool operator()(const WORDTYPE* a, const WORDTYPE* b) const {
//    while (*a && *b) {
//      if (*a != *b) 
//        return 0;
//      a++;
//      b++;
//    }
//    return (*a == *b);
//  }
//};

// deep hash of zero-terminated string of ints
//struct hashphrase {
//  size_t operator()(const WORDTYPE *s) const {	// hash 0-terminated string of shorts
//    const WORDTYPE * p;
//    unsigned int h;	// assumes 32-bit int
//    h = 0;
//    for (p = s; *p; p++) {
//      // treat it as an integer
//      int t = *p;
//      h = (h << 4) + t;
//      h = (h ^ ((h & 0xf0000000) >> 24)) & 0x0fffffff;
//    }
//
//    return h;
//  }
//};

struct hashphrase
{
    size_t operator()(const vector<int>& s) const
    {
        unsigned int h=0;
        for(unsigned int i=0;i<s.size();i++)
        {
            int word_index=s[i];
            h=(h<<4)+word_index;
            h = (h ^ ((h & 0xf0000000) >> 24)) & 0x0fffffff;
        }
        return h;
    }
};

struct eqphrase
{
    bool operator()(const vector<int>& a,const vector<int>& b) const
    {
        if(a.size()!=b.size())
            return 0;
        for(unsigned int i=0;i<a.size();i++)
        {
            if(a[i]!=b[i])
                return 0;
        }
        return 1;
    }
};

struct Phrase_entry :std::unordered_map<vector<int>,PhraseRule*,hashphrase,eqphrase>
{
	
};

struct Phrase_table :std::unordered_map<vector<int>,Phrase_entry,hashphrase,eqphrase>
{

};

#endif
