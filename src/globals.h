#ifndef Global_H
#define Global_H
#include <unordered_map>
#include <string>
typedef int WORDTYPE;
#define MAXN 4
using namespace std;

struct Tran_entry : unordered_map<int,double>
{

};
struct Tran_table : unordered_map<int,Tran_entry>
{

};

struct Dict_entry: unordered_map<string,int>
{

};
struct Dict_table: unordered_map<string,Dict_entry>
{

};
#endif
