#ifndef LEXANALYSIS_H
#define LEXANALYSIS_H

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <stdio.h>
#include <string.h>
#include <algorithm>
using namespace std;
class LexAnalysis
{
public:
	int lineNum = 1;
	char ch; //last char read
	string sym; //last word symbol
	vector<char> word;
	ifstream in;
	ofstream out;

	void ToUpperString(string &str);
	void Lex_init(char *str);
	void _error(int t);
	void _getchar();
	void _ident();
	void _char();
	void _string();
	void _number();
	bool _letter(char c);
	bool _ifSepa(char c);
	void _separator();
	void _analysis();
	bool _ifSaved(string str);
	void _output(string s, string str);
	string _getword();

};



#endif // LEXANALYSIS_H
