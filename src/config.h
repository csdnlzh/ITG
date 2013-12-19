#ifndef Config_H

#define Config_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include "configFile.h"
#include "tools.h"

using namespace std;
namespace ITG{

class Config
{

public:
	static Config& GetSingleton()
	{
        if(!instance)
        {
            instance=new Config();
        }
		return *instance;
	}
	ConfigFile* configfile;
	
	bool pruning;
	string out_dir;
private:

	static ITG::Config* instance;

	Config()
	{
         cout<<"begin config"<<endl;
		 configfile=new ConfigFile("config.txt");
		 pruning= configfile->read<int>("pruning")==1 ? true: false;
		 out_dir=configfile->read<string>("out_path");

         cout<<"config ok"<<endl;
	};

};

}//namespace ITG

#endif

