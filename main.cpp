#include <iostream>
#include <fstream>
#include <stdio.h>
#include <vector>
#include <string>
#include "table.h"
#include "inter_code.h"
#include "mips_code.h"
#include "LexAnalysis.h"

using namespace std;
ifstream in;
ofstream out;

/*
1. 在每个判断cout输出这是什么语句前进行符号表判断，然后清空记录.要在getsym之前，不然会读入下一语句的单词

统计局部变量大小
情况语句
*/



string lineNum="0";
string values;	//整个字符
string sym;
char value;		//首字符
char line[256];
//用于填、查表操作
int level = 0;
vector<string> table_sym;
vector<string> table_values;
int i = 0;

//用于中间代码生成
vector<string> inter_sym;
vector<string> inter_values;

//用于判断返回值
bool inBlock = false;	//判断是否在if,while,switch语句中
bool existReturn = false;	//当inBlock = false时遇到return语句，existReturn = true

void error(int type);	//错误处理
void getsym();	//获取下个单词类型
void _const();	//常量声明
void _var();	//变量声明
void _funcDefWithArg();	//有返回值函数定义
void _funcDefNoArg();	//无返回值函数定义
void _main();	//主函数
void _statement();	//语句
void _assign();	//赋值语句
void _if();		//条件语句
void _while();	//循环语句
void _switch();	//情况语句
void _scanf();	//读语句
void _print();	//写语句
void _return();	//返回语句
void _funcCall();	//函数调用语句
void _null();	//空语句
void _staList();	//语句列
void _compState();	//复合语句
void _expression();	//表达式
void _factor();	//因子
void _item();		//项
void _args();	//参数
void _conditions();	//条件
int _caseList(bool first, int ob_id, int table_num);	//情况表
void _program();	//程序


int main() {

	LexAnalysis lexAnalysis;

	cout << "Please input the data path:" << endl;
	char filepath[200];
	cin >> filepath;

	lexAnalysis.Lex_init(filepath);

	in.open("out.txt");
	if (!in.is_open())
	{
		out << "Error opening file" << endl;
		exit(1);
	}
	remove("SynAnalresult.txt");
	out.open("SynAnalresult.txt");
	//打开interCode.txt
	inter_init();
	getsym();
	_program();
	mips_init();
 	system("pause");
	return 0;
}


bool var_mark = true;	//变量说明是否还合法
string temp_sym, temp_values;
/*
全局变量中的var难以解决，放在inter_code里判断吧。
*/
void _program() {
	if (sym == "CONSTSYM") {
		_const();
		JToMain();
		if (sym != "CONSTSYM") {
			_program();
		}
		else {
			error(0);
			_program();
		}
	}
	else if (sym == "INTSYM" || sym == "CHARSYM") {
		getsym();
		if (sym == "IDENT") {
			getsym();
			if (sym == "LPAREN") {
				var_mark = false;
				JToMain();
				_funcDefWithArg();
				if (sym != "CONSTSYM") {
					_program();
				}
				else {
					error(0);
					_program();
				}
			}
			else {
				if (var_mark) {
					var_mark = false;
					_var();


					if (sym != "CONSTSYM") {
						_program();
					}
					else {
						error(0);
						_program();
					}
				}
				else {
					error(0);
					_program();
				}
			}
		}
		else {
			error(0);
		}
	}
	else if (sym == "VOIDSYM") {
		JToMain();
		getsym();
		if (sym == "MAINSYM") {
			_main();
			table_sym.clear();
			table_values.clear();
			out << "This is a program" << endl;
		}
		else if (sym == "IDENT") {
			var_mark = false;
			_funcDefNoArg();

			if (sym != "CONSTSYM") {
				_program();
			}
			else {
				error(0);
				_program();
			}
		}
		else {
			error(0);
			_program();
		}
	}
	else {
		error(0);
		_program();
	}
}


//在这里根据分函数判断的结果和最后的;判断是否是一个正常的语句
void _statement() {
	vector<string> temp;
	int size = 0;
	if (sym == "LBRACE") {
		table_sym.clear();
		table_values.clear();

		getsym();
		if (sym == "RBRACE") {
			table_sym.clear();
			table_values.clear();
			out << "This is a statement list" << endl;
			getsym();
		}
		else {
			_staList();
			if (sym == "RBRACE") {
				table_sym.clear();
				table_values.clear();
				out << "This is a statement list" << endl;
				getsym();
			}
			else {
				error(6);
			}
		}

	}
	else if(sym == "SWITCHSYM"){
		inBlock = true;
		_switch();
		inBlock = false;
	}
	else if (sym == "IFSYM") {
		inBlock = true;
		_if();
		inBlock = false;
	}
	else if (sym == "WHILESYM") {
		inBlock = true;
		_while();
		inBlock = false;
	}
	else {
		if (sym == "IDENT") {
			getsym();
			if (sym == "LPAREN") {
				//因为表达式中也会调用到_funcCall()，所以不适合在函数里面调用翻译部分
				_funcCall();

				//翻译funcCall
				T_num = 0;
				W_i = 0;
				funcCall_tra(inter_sym, inter_values);
				inter_sym.clear();
				inter_values.clear();
			}
			else {
				//去除assign语句和_funcCall语句前面的杂单词
				size = inter_sym.size();
				temp.push_back(inter_sym[size - 1]);
				temp.push_back(inter_values[size - 1]);
				temp.push_back(inter_sym[size - 2]);
				temp.push_back(inter_values[size - 2]);
				inter_sym.clear();
				inter_values.clear();
				inter_sym.push_back(temp[2]);
				inter_sym.push_back(temp[0]);
				inter_values.push_back(temp[3]);
				inter_values.push_back(temp[1]);
				temp.clear();
				_assign();
			}
		}
		else if (sym == "SCANFSYM") {
			_scanf();
		}
		else if (sym == "PRINTFSYM") {
			_print();
		}
		else if (sym == "RETURNSYM") {
			if (!inBlock) {
				existReturn = true;
			}
			_return();
		}
		else if (sym == "SEMICOLON") { //这个要特殊处理一下
			_null();
		}
		else {
			error(0);
		}
		//判断结尾是否为;
		if (sym == "SEMICOLON") {
			//检查语句中标识符是否合法
			use_check(lineNum, level, table_sym, table_values);
			table_sym.clear();
			table_values.clear();
			getsym();
		}
		else {
			error(2);
		}
	}
}

void getsym() {
	int i = 0;
	sym = "";
	values = "";
	if (!in.eof()) {
		in.getline(line, 256);
		while (line[i] != ' ') {
			sym += line[i];
			i++;
		}
		while (line[i] == ' ') {
			i++;
		}
		value = line[i];
		while (line[i] != '\0') {
			values += line[i];
			i++;
		}
		if (sym == "line:") {
			lineNum = values;
			getsym();
		}
		else {
			table_sym.push_back(sym);
			table_values.push_back(values);

			inter_sym.push_back(sym);
			inter_values.push_back(values);
		}
	}
	else
	{
		exit(0);
	}
}

void _const() {
	getsym();
	string type = sym;
	if (sym == "INTSYM" || sym == "CHARSYM") {
		getsym();
		if (sym == "IDENT") {
			getsym();
			if (sym == "EQL") {
				getsym();
				if ((type == "INTSYM" && sym == "NUMBER") || (type == "CHARSYM" && sym == "CHAR")) {
					getsym();
					while (sym == "COMMA") {
						getsym();
						if (sym == "IDENT") {
							getsym();
							if (sym == "EQL") {
								getsym();
								if ((type == "INTSYM" && sym == "NUMBER") || (type == "CHARSYM" && sym == "CHAR")) {
									getsym();
								}
								else {
									error(11);
									break;
								}
							}
							else {
								error(14);
								break;
							}
						}
						else {
							error(12);
							break;
						}
					}
				}
				else {
					error(11);
				}
			}
			else {
				error(14);
			}
		}
		else {
			error(12);
		}
	}
	else {
		error(13);
	}

	if (sym == "SEMICOLON") {
		if (!define_check(lineNum,level, table_values)) {
			addTable(level, table_values);
		}
		table_sym.clear();
		table_values.clear();

		getsym();
		if (sym == "CONSTSYM") {
			_const();
		}
		else {
			//翻译const
			temp_sym = inter_sym.back();
			temp_values = inter_values.back();
			inter_sym.pop_back();
			inter_values.pop_back();
			T_num = 0;
			W_i = 0;
			globVar_deal(inter_values);
			inter_sym.clear();
			inter_values.clear();
			inter_sym.push_back(temp_sym);
			inter_values.push_back(temp_values);

			out << "This is a const define" << endl;
		}

	}
	else {
		error(2);
	}
}

void _var() {
	string temp_inter;
	if (sym == "SEMICOLON") {
		if (!define_check(lineNum,level, table_values)) {
			addTable(level, table_values);
		}
		table_sym.clear();
		table_values.clear();

		getsym();
		if (sym == "INTSYM" || sym == "CHARSYM") {
			getsym();
			if (sym == "IDENT") {
				temp_inter = values;
				getsym();
				if (sym == "LPAREN") {
					out << "This is a var define" << endl;

					_funcDefWithArg();
				}
				else {
					_var();

				}
			}
			else {
				error(12);
			}
		}
		else {
			out << "This is a var define" << endl;
		}

	}
	else if (sym == "LBRACK") {
		getsym();
		if (sym == "NUMBER" && (value > '0' && value <= '9')) {		//无符号整数
			getsym();
			if (sym == "RBRACK") {
				getsym();
				_var();
			}
			else {
				error(7);
			}
		}
		else {
			error(15);
		}
	}
	else if(sym == "COMMA"){
		while (sym == "COMMA") {
			getsym();
			if (sym == "IDENT") {
				getsym();
				_var();
			}
			else {
				error(12);
			}
		}
	}
	else {
		error(0);
	}
}
void _funcDefWithArg(){
	inBlock = false;
	existReturn = false;
	level++;
	getsym();
	if (sym != "RPAREN") {
		_args();
	}
	if (sym == "RPAREN") {
		if (!define_check(lineNum,level, table_values)) {
			addTable(level, table_values);
		}
		table_sym.clear();
		table_values.clear();

		//翻译
		T_num = 0;
		W_i = 0;
		func_tra(inter_sym, inter_values);
		inter_sym.clear();
		inter_values.clear();

		getsym();
		if (sym == "LBRACE") {
			table_sym.clear();
			table_values.clear();

			getsym();
			if (sym == "RBRACE") {
				table_sym.clear();
				table_values.clear();
				add_func_localvar_size();
				//判断外层是否有return
				if (!existReturn) {
					error(16);
				}

				out << "This is a function with return define" << endl;

				/*func_tra(inter_sym,inter_values);
				inter_sym.clear();
				inter_values.clear();*/

				getsym();
			}
			else {
				_compState();
				if (sym == "RBRACE") {
					table_sym.clear();
					table_values.clear();
					add_func_localvar_size();
					//判断外层是否有return
					if (!existReturn) {
						error(16);
					}
					out << "This is a function with return define" << endl;

					/*func_tra(inter_sym, inter_values);
					inter_sym.clear();
					inter_values.clear();*/

					getsym();
				}
				else {
					error(6);
				}
			}
		}
		else {
			error(5);
		}
	}
	else {
		error(4);
	}
}
void _funcDefNoArg() {
	level++;
	getsym();
	if (sym == "LPAREN") {
		getsym();
		if (sym != "RPAREN") {
			_args();
		}
		if (sym == "RPAREN") {

			if (!define_check(lineNum,level, table_values)) {
				addTable(level, table_values);
			}
			table_sym.clear();
			table_values.clear();

			//翻译
			T_num = 0;
			W_i = 0;
			func_tra(inter_sym, inter_values);
			inter_sym.clear();
			inter_values.clear();


			getsym();
			if (sym == "LBRACE") {
				table_sym.clear();
				table_values.clear();

				getsym();
				if (sym == "RBRACE") {
					table_sym.clear();
					table_values.clear();

					add_func_localvar_size();
					out << "This is a function with no define" << endl;

					/*func_tra(inter_sym, inter_values);
					inter_sym.clear();
					inter_values.clear();*/

					getsym();
				}
				else {
					_compState();
					if (sym == "RBRACE") {
						table_sym.clear();
						table_values.clear();

						add_func_localvar_size();
						out << "This is a function with no define" << endl;

						/*func_tra(inter_sym, inter_values);
						inter_sym.clear();
						inter_values.clear();*/

						getsym();
					}
					else {
						error(6);
					}
				}
			}
			else {
				error(5);
			}
		}
		else {
			error(4);
		}
	}
	else {
		error(3);
	}
}
void _main() {
	level++;
	getsym();
	if (sym == "LPAREN") {
		getsym();
		if (sym == "RPAREN") {
			if (!define_check(lineNum,level, table_values)) {
				addTable(level, table_values);
			}
			table_sym.clear();
			table_values.clear();

			//翻译
			T_num = 0;
			W_i = 0;
			func_tra(inter_sym, inter_values);
			inter_sym.clear();
			inter_values.clear();

			getsym();
			if (sym == "LBRACE") {
				table_sym.clear();
				table_values.clear();

				getsym();
				if (sym == "RBRACE") {
					table_sym.clear();
					table_values.clear();

					add_func_localvar_size();
					out << "This is a main function" << endl;

					/*func_tra(inter_sym, inter_values);
					inter_sym.clear();
					inter_values.clear();*/

					getsym();
				}
				else {
					_compState();
					if (sym == "RBRACE") {
						table_sym.clear();
						table_values.clear();

						add_func_localvar_size();
						out << "This is a main function" << endl;

						/*func_tra(inter_sym, inter_values);
						inter_sym.clear();
						inter_values.clear();*/

						getsym();
					}
					else {
						error(6);
					}
				}
			}
			else {
				error(5);
			}
		}
		else {
			error(4);
		}
	}
	else {
		error(3);
	}
}

void _assign() {
	int j = 0;
	if (sym == "EQL") {
		getsym();
		_expression();
	}
	else if (sym == "LBRACK") {
		getsym();
		_expression();
		if (sym == "RBRACK") {
			getsym();
			if (sym == "EQL") {
				getsym();
				_expression();
				out << "This is a assign statement" << endl;
			}
			else {
				error(14);
			}
		}
		else {
			error(8);
		}
	}
	else {
		error(0);
	}
	//翻译
	T_num = 0;
	W_i = 0;
	assign_tra(inter_sym, inter_values);
	inter_sym.clear();
	inter_values.clear();
}
void _if() {
	int table_num = 0;
	getsym();
	if (sym == "LPAREN") {
		getsym();
		_conditions();
		if (sym == "RPAREN") {

			//检查语句中标识符是否合法
			use_check(lineNum, level, table_sym, table_values);
			table_sym.clear();
			table_values.clear();

			//生成if头
			T_num = 0;
			W_i = 0;
			table_num = if_begin_tra(inter_sym, inter_values);
			inter_sym.clear();
			inter_values.clear();


			getsym();
			_statement();

			//生成if尾
			T_num = 0;
			W_i = 0;
			table_num = if_end_tra(table_num);

			if (sym == "ELSESYM") {
				table_sym.clear();
				table_values.clear();
				getsym();
				_statement();
			}
			else_end_tra(table_num);
			out << "This is a if statement" << endl;
		}
		else {
			error(4);
		}
	}
	else {
		error(3);
	}
}
void _while() {
	int table_num = 0;
	getsym();
	if (sym == "LPAREN") {
		getsym();
		_conditions();
		if (sym == "RPAREN") {

			//检查语句中标识符是否合法
			use_check(lineNum, level, table_sym, table_values);
			table_sym.clear();
			table_values.clear();

			//生成While头
			T_num = 0;
			W_i = 0;
			table_num = while_begin_tra(inter_sym, inter_values);
			inter_sym.clear();
			inter_values.clear();

			getsym();
			_statement();

			//生成while尾
			T_num = 0;
			W_i = 0;
			while_end_tra(table_num);

			out << "This is a while statement" << endl;
		}
		else {
			error(4);
		}
	}
	else {
		error(3);
	}
}
void _switch() {
	int ob_id = 0;
	int table_num = 0;
	getsym();
	if (sym == "LPAREN") {
		getsym();
		_expression();
		if (sym == "RPAREN") {

			//检查语句中标识符是否合法
			use_check(lineNum, level, table_sym, table_values);
			table_sym.clear();
			table_values.clear();

			T_num = 0;
			W_i = 0;
			ob_id = switch_begin_tra(inter_sym,inter_values);
			inter_sym.clear();
			inter_values.clear();

			getsym();
			if (sym == "LBRACE") {
				table_sym.clear();
				table_values.clear();

				getsym();
				table_num = _caseList(true,ob_id,0);

				default_tra(table_num);
				inter_sym.clear();
				inter_values.clear();
				if (sym == "DEFAULTSYM") {
					getsym();
					if (sym == "COLON") {
						getsym();
						_statement();
					}
					else {
						error(9);
					}
				}
				if (sym == "RBRACE") {
					table_sym.clear();
					table_values.clear();

					switch_end_tra();

					out << "This is a switch statement" << endl;
					getsym();
				}
				else {
					error(6);
				}
			}
			else {
				error(5);
			}
		}
		else {
			error(4);
		}
	}
	else {
		error(3);
	}
}
void _scanf() {
	getsym();
	if (sym == "LPAREN") {
		getsym();
		if (sym == "IDENT") {
			getsym();
			while (sym == "COMMA") {
				getsym();
				if (sym == "IDENT") {
					getsym();
				}
				else {
					error(12);
					break;
				}
			}
		}
		else {
			error(12);
		}

		if (sym == "RPAREN") {
			//翻译
			T_num = 0;
			W_i = 0;
			scanf_tra(inter_sym,inter_values);
			inter_sym.clear();
			inter_values.clear();

			out << "This is a scanf statemenrt" << endl;
			getsym();
		}
		else {
			error(4);
		}

	}
	else {
		error(3);
	}
}
void _print() {
	getsym();
	if (sym == "LPAREN") {
		getsym();
		if(sym == "STRING"){
			getsym();
			if (sym == "COMMA") {
				getsym();
				_expression();
			}
		}
		else {
			_expression();
		}

		if (sym == "RPAREN") {
			//翻译
			T_num = 0;
			W_i = 0;
			printf_tra(inter_sym, inter_values);
			inter_sym.clear();
			inter_values.clear();

			out << "This is a printf statemenrt" << endl;
			getsym();
		}
		else {
			error(4);
		}
	}
	else {
		error(3);
	}
}
void _return() {
	getsym();
	if (sym != "SEMICOLON") {
		if (sym == "LPAREN") {
			getsym();
			if (sym != "RPAREN") {
				_expression();
			}
			if (sym == "RPAREN") {
				out << "This is a return statemenrt" << endl;
				getsym();
			}
			else {
				error(4);
			}
		}
		else {
			error(3);
		}
	}
	//翻译return
	T_num = 0;
	W_i = 0;
	return_tra(inter_sym, inter_values);
	inter_sym.clear();
	inter_values.clear();
}
void _funcCall() {
	getsym();
	if (sym == "RPAREN") {
		out << "This is a function call statement" << endl;
		getsym();
	}
	else {
		_expression();
		while (sym == "COMMA") {
			getsym();
			if (sym != "RPAREN") {
				_expression();
			}
		}
		if (sym == "RPAREN") {
			out << "This is a function call statement" << endl;
			getsym();
		}
		else {
			error(4);
		}
	}
}
void _null() {
	out << "This is a null statement" << endl;
}
void _staList() {
	while (sym != "RBRACE") {
		_statement();
	}
	out << "This is a statement list" << endl;
}

void _expression() {
	if (sym == "MINUS" || sym == "PLUS") {
		getsym();
	}
	_item();
	while (sym == "MINUS" || sym == "PLUS") {
		getsym();
		_item();
	}
}

void _item() {
	_factor();
	while (sym == "MULT" || sym == "DIV") {
		getsym();
		_factor();
	}
}

void _factor() {
	vector<string> temp;
	int size = 0;
	if (sym == "IDENT") {
		getsym();
		if (sym == "LBRACK") {
			getsym();
			if (sym != "RBRACK") {
				_expression();
			}
			if (sym == "RBRACK") {
				getsym();
			}
			else {
				error(8);
			}
		}
		else if (sym == "LPAREN") {
			_funcCall();
		}
	}
	else if (sym == "NUMBER" || sym == "CHAR") {
		getsym();
	}
	else if (sym == "LPAREN") {
		getsym();
		if (sym != "RPAREN") {
			_expression();
		}
		if (sym == "RPAREN") {
			getsym();
		}
		else {
			error(4);
		}
	}
	else {
		error(0);
	}
}


//不出理空的情况
void _compState() {
	if (sym == "CONSTSYM") {
		_const();
		if (sym == "INTSYM" || sym == "CHARSYM") {
			getsym();
			if (sym == "IDENT") {
				getsym();
				_var();
				if (sym != "RBRACE") {
					_staList();
				}
			}
			else {
				error(12);
			}
		}
		else {
			if (sym != "RBRACE") {
				_staList();
			}
		}
	}
	else if (sym == "INTSYM" || sym == "CHARSYM") {
		getsym();
		if (sym == "IDENT") {
			getsym();
			_var();
			if (sym != "RBRACE") {
				_staList();
			}
		}
		else {
			error(12);
		}
	}
	else {
		_staList();
	}
}

void _conditions() {
	_expression();
	if (sym == "DEQL" || sym == "NEQ" || sym == "LEQ" || sym == "LSS" || sym == "GEQ" || sym == "GTR") {
		getsym();
		_expression();
	}
}

void _args() {
	if (sym == "INTSYM" || sym == "CHARSYM") {
		getsym();
		if (sym == "IDENT") {
			getsym();
			while (sym == "COMMA") {
				getsym();
				if (sym == "INTSYM" || sym == "CHARSYM") {
					getsym();
					if (sym == "IDENT") {
						getsym();
					}
					else {
						error(12);
					}
				}
				else {
					error(13);
				}
			}
		}
		else {
			error(12);
		}
	}
	else {
		error(13);
	}
}

int _caseList(bool first, int ob_id, int table_num) {
	int temp_table_num = 0;
	int result = 0;
	int temp = 0;
	if (sym == "CASESYM") {
		getsym();
		if (sym == "NUMBER" || sym == "CHAR") {
			getsym();
			if (sym == "COLON") {
				//翻译case
				W_i = 0;
				temp_table_num = case_tra(first, ob_id, table_num, inter_sym, inter_values);
				inter_sym.clear();
				inter_values.clear();
				if (result < temp_table_num) {
					result = temp_table_num;
				}
				getsym();
				_statement();
				out << "This is a case statement" << endl;
				if (sym == "CASESYM") {
					temp = _caseList(false, ob_id, temp_table_num);
					if (result < temp) {
						result = temp;
					}
				}
			}
			else {
				error(9);
			}
		}
		else {
			error(11);
		}
	}
	else {
		error(10);
	}
	return result;
}
void error(int type) {


	string str = "line: " + lineNum;
	out << str << endl;
	switch (type) {
		case 0:
			out << "ERROR : ";
			out << values << endl;
			getsym();
			break;
		case 1:
			out << "ERROR : lose ',' before ";
			out<< values <<endl;
			break;
		case 2:
			out << "ERROR : lose ';' before ";
			out << values << endl;
			break;
		case 3:
			out << "ERROR : lose '(' before " ;
			out << values << endl;
			break;
		case 4:
			out << "ERROR : lose ')' before " ;
			out << values << endl;
			break;
		case 5:
			out << "ERROR : lose '{' before " ;
			out << values << endl;
			break;
		case 6:
			out << "ERROR : lose '}' before ";
			out << values << endl;
			break;
		case 7:
			out << "ERROR : lose '[' before ";
			out << values << endl;
			break;
		case 8:
			out << "ERROR : lose ']' before ";
			out << values << endl;
			break;
		case 9:
			out << "ERROR : lose ':' before ";
			out << values << endl;
			break;
		case 10:
			out << "ERROR : lose 'case' before ";
			out << values << endl;
			break;
		case 11:
			out << values;
			out << "is ";
			out << "ERROR : not a number or char" << endl;
			break;
		case 12:
			out << values;
			out << "is ";
			out << "ERROR : not an ident" << endl;
			break;
		case 13:
			out << values;
			out << "is ";
			out << "ERROR : an undefined type" << endl;
			break;
		case 14:
			out << "ERROR : ";
			out << values;
			out << "is ";
			out << "not a '='" << endl;
			break;
		case 15:
			out << "ERROR : ";
			out << values;
			out << "is ";
			out << "not a unsigned number" << endl;
			break;
		case 16:
			out << "ERROR : lose return" << endl;
			break;
		default:
			out << "ERROR : ";
			out << values;
			out << "is ";
			out<< "unrecognized word" << endl;
			break;
	}
}


