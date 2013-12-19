#ifndef Symbols_H

#define Symbols_H
#include <string>
#include <unordered_map>
using namespace std;

class Symbols
{
public:
	static std::unordered_map<string,int> symbols;
	static std::unordered_map<int,string> invert_symbols;
	static void InitSymbols();

};

#endif
