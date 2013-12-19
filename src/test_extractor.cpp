#include <fstream>
#include "extractor.h"
#include "configFile.h"
#include "config.h"
#include "corpus.h"
#include "loger.h"
#include "alignment.h"

using namespace std;


int main2()
{
	Loger::log("starting...",true);
	Corpus& corpus =Corpus::GetSingleton();
	corpus.ReadFile();
	ConfigFile* config=ITG::Config::GetSingleton().configfile;
	string afilename=config->read<string>("afile");
	vector<Alignment> alignments;
	AlignmentBuilder align_builder;
	align_builder.BuildAlignments(alignments,afilename);

	Extractor extractor;
	extractor.Run(alignments);
	

	return 0;
}
