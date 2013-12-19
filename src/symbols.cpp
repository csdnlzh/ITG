#include "symbols.h"

unordered_map<string,int> Symbols::symbols;
unordered_map<int,string> Symbols::invert_symbols;
void Symbols::InitSymbols()
{
	symbols["S"]=0;
	symbols["A"]=1;
	symbols["B"]=2;
	symbols["C"]=3;

	invert_symbols[0]="S";
	invert_symbols[1]="A";
	invert_symbols[2]="B";
	invert_symbols[3]="C";
}
