#include "loger.h"
#include "tools.h"
#include<string>
#include<fstream>
#include<time.h>
#include<iomanip>
#include<iostream>
using namespace std;

ofstream Loger::mylogfile("log.txt",ios::app);
FILE* Loger::logfile=fopen("train_log.txt","w");

void Loger::init()
{

}
void Loger::log(vector<string> msgs,bool timeflag)
{
	if(timeflag)
	{
		time_t lt;
		lt =time(NULL);
		mylogfile<<ctime(&lt);
	}
    for(unsigned int i=0;i<msgs.size();i++)
	{
		mylogfile<<msgs[i]<<endl;
	}
	mylogfile<<endl;
}

void Loger::log(string msg,bool timeflag)
{
	if(timeflag)
	{
		time_t lt;
		lt =time(NULL);
		mylogfile<<ctime(&lt);
	}
	mylogfile<<msg<<endl<<endl;
};
void Loger::log(double msg,bool timeflag)
{
	if(timeflag)
	{
		time_t lt;
		lt =time(NULL);
		mylogfile<<ctime(&lt);
	}
	mylogfile<<msg<<endl<<endl;
};

void Loger::log(vector<double> msgs,bool timeflag)
{
	if(timeflag)
	{
		time_t lt;
		lt =time(NULL);
		mylogfile<<ctime(&lt);
	}
	for(unsigned int i=0;i<msgs.size();i++)
	{
		mylogfile<<setprecision(20)<<msgs[i]<<endl;
	}
	mylogfile<<endl;
}

void Loger::LogTime()
{
	time_t lt;
	lt =time(NULL);
	mylogfile<<ctime(&lt);
}
