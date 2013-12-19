#ifndef Tools_H
#define Tools_H
#include <string>
#include <fstream>
#include <vector>
#include"globals.h"
using namespace std;

class Tools
{

public:

	static string d2str(double d);

	static string i2str(int i);

	static long long getHashkey(int lhs,int rhsl,int rhsr,int type);

	static int phrase_length(const WORDTYPE* phrase);

	//from haozhang itg code
	static double digamma(double x) ;

	static string  GetFileFullName(const string& fileName);
    
    static string GetTime();
};

void fprint_array(FILE *where,const vector<WORDTYPE>& v);
double myexp(double x);
#endif
