#include <sstream>
#include <boost/algorithm/string.hpp>
#include <stdio.h>
#include <time.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include "tools.h"
#include "config.h"
using namespace std;

string Tools::d2str(double d)
{
	ostringstream ostr;
	ostr<<setprecision(6)<<d;
	return ostr.str();
}

string Tools::i2str(int i)
{
	ostringstream ostr;
	ostr<<i;
	return ostr.str();
}

long long Tools::getHashkey(int lhs,int rhsl,int rhsr,int type)
{
	return  ((long long)lhs<<48)+ ((long long)rhsl<<32)+((long long)rhsr<<16)+(type); 
}

int Tools::phrase_length(const WORDTYPE* phrase)
{
	int len=-1;
	while(phrase[++len]);
	return len;
}

//from haozhang itg code
double Tools::digamma(double x) 
{
	double result = 0, xx, xx2, xx4;

	for ( ; x < 7; ++x)
	result -= 1/x;
	x -= 1.0/2.0;
	xx = 1.0/x;
	xx2 = xx*xx;
	xx4 = xx2*xx2;
	result += log(x)+(1./24.)*xx2-(7.0/960.0)*xx4+(31.0/8064.0)*xx4*xx2-(127.0/30720.0)*xx4*xx4;
	return result;
}
/* prints an array to the specified file stream */
void fprint_array(FILE *where, const vector<WORDTYPE>& v) {
  for(unsigned int i=0;i<v.size();i++)
    fprintf(where, "%d ", v[i]);
}

string  Tools::GetFileFullName(const string& fileName)
{
	string out_dir=ITG::Config::GetSingleton().out_dir;
	string stage = ITG::Config::GetSingleton().configfile->read<string>("stage");
	string full_filename= out_dir+"/"+stage+fileName;
	return full_filename;
}

string Tools::GetTime()
{
    time_t lt;
    lt=time(NULL);
    return string(ctime(&lt));
}
