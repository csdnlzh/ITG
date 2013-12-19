#ifndef Phrase_Pair_H
#define Phrase_Pair_H

class PhrasePair
{
public:
	PhrasePair(int es,int ee,int cs,int ce)
	{
		estart=es;
		eend=ee;
		cstart=cs;
		cend=ce;
	}
	int estart;
	int eend;
	int cstart;
	int cend;
};
#endif