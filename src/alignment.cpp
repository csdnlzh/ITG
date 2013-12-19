#include "alignment.h"
#include "tools.h"
#include <fstream>
#include <iostream>
using namespace std;
/**
 * @file alignment.cpp
 * @brief Implement file for the AlignmentBuilder. Read a alignment file and build alignment objects.
 * @author lizhonghua
 * @version 1.0
 * @date 2012-03-29
 */




/**
 * @brief  Read alignment file and build alignment objects.
 *
 * @param alignments  a vector for storing all the alignment objects
 * @param afilename   alignment file
 */
void AlignmentBuilder::BuildAlignments(vector<Alignment>& alignments,string afilename)
{
    cout<<"read alignment file and build alignment: "<<afilename<<endl;
	ifstream afile(afilename);
	if(!afile)
	{
		cout<<"open alignment file error! "<<endl;
	}

	while(!afile.eof())
	{
		string line;
		getline(afile,line);
		Alignment align(line);
		alignments.push_back(align);
	}

    cout<<"build alignment ok!"<<endl;
}
