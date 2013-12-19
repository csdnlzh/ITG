#ifndef Loger_H

#define Loger_H

#include<string>

#include<vector>

#include <fstream>

#include <ostream>

using namespace std;



enum LogerType{file,std_out};

class Loger
{

public:
	static void log(double msg,bool timeflag=true);

	static void log(string msg,bool timeflag=true);

	static void log(vector<string> msgs,bool timeflag=true);

	static void log(vector<double> msgs,bool timeflag=true);

	static void LogTime();

	static ofstream mylogfile;

	static FILE* logfile;
    
    static void init();

};



#endif
