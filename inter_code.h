#ifndef INTER_CODE_H_INCLUDED
#define INTER_CODE_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>
#include "table.h"

using namespace std;

ofstream inter_out;

int if_lable_num = 0;
int while_lable_num = 0;
int case_lable_num = 0;
int switch_end_label_num = 0;

/*
���ⲿ����expression_traʱ����
������¼�����һ�����ʽ�Ĺ������õ�����ʱ���������������������T_num��
*/
int T_num = 0;
int W_i = 0;

void inter_init();
void JToMain();
void output_inter(string op, string arg1, string arg2, string result);
int expression_tra(vector<string> &sym, vector<string> &values);
int item_tra(vector<string> &sym, vector<string> &values);
int factor_tra(vector<string> &sym, vector<string> &values);


void func_tra(vector<string> &sym, vector<string> &values);
void globVar_deal(vector<string> &values);
int if_begin_tra(vector<string> &sym, vector<string> &values);
int if_end_tra(int table_num);
void else_end_tra(int table_num);
int switch_begin_tra(vector<string> &sym, vector<string> &values);
int case_tra(bool first, int ob, int table_num, vector<string> &sym, vector<string> &values);
void default_tra(int table_num);
void switch_end_tra();
int while_begin_tra(vector<string> &sym, vector<string> &values);
void while_end_tra(int table_num);
void assign_tra(vector<string> &sym, vector<string> &values);
void scanf_tra(vector<string> &sym, vector<string> &values);
void printf_tra(vector<string> &sym, vector<string> &values);
void const_tra(vector<string> &values);
int funcCall_tra(vector<string> &sym, vector<string> &values);
void return_tra(vector<string> &sym, vector<string> &values);

void inter_init() {
	remove("interCode.txt");
	inter_out.open("interCode.txt");
}
/*
int/char/void func(........)
*/
void func_tra(vector<string> &sym, vector<string> &values) {
	while (!(sym[W_i] == "VOIDSYM" || ((sym[W_i] == "INTSYM" || sym[W_i] == "CHARSYM") && sym[W_i + 1] == "IDENT" && sym[W_i+2] == "LPAREN"))) {
		W_i++;
	}

	//�ӱ�ǩ
	string func_name = values[W_i+1];
	inter_out << "LABEL_FUNC_" + func_name +":"<< endl;
}


void globVar_deal(vector<string> &values) {
	while (values[W_i] != "const") {
		W_i++;
	}

	int size = values.size();
	vector<string> temp;
	if (values[W_i] == "const") {
		while (W_i < size) {
			temp.clear();
			while (values[W_i] != ";") {
				temp.push_back(values[W_i]);
				W_i++;
			}
			temp.push_back(values[W_i]);
			const_tra(temp);
			W_i++;
		}
	}
}

/*
const int/char a = 1[, b = 2];
*/
void const_tra(vector<string> &values) {
	int j = 1;
	int size = values.size();
	int number = 0;
	char c;
	if (values[j] == "int") {
		j++;
		while (j < size) {
			output_inter("=","(" + values[j + 2] + ")","", values[j]);
			j += 4;
		}
	}
	else {
		j++;
		while (j < size) {
			c = values[j + 2][1];
			output_inter("=", "(" + to_string(c - '0' + 48) + ")", "", values[j]);
			j += 4;
		}
	}
}

/*
if(<����>)
<����> ::= �����ʽ�� (����ϵ������������ʽ�������գ�)
���ض�Ӧ��table���

ռ������ǩ����Ϊif�������ʱҲҪ����else�����ĵط�
*/
int if_begin_tra(vector<string> &sym,vector<string> &values) {
	//ȥ��
	while (sym[W_i] != "IFSYM") {
		W_i++;
	}
	int res1 = 0, res2 = 0;
	string op = "";
	W_i = W_i +2;
	T_num = 0;
	res1 = expression_tra(sym, values);
	if (values[W_i] == ")") {
		output_inter("=", "(0)", "", "T_" + to_string(T_num));
		res2 = T_num;
		T_num++;
		output_inter("==", "T_" + to_string(res1), "T_" + to_string(res2),"LABEL_IF_"+ to_string(if_lable_num));
		if_lable_num+=2;
	}
	else {
		op = values[W_i];
		W_i++;
		res2 = expression_tra(sym, values);
		//��op
		if (op == "==") {
			op = "!=";
		}
		else if (op == "!=") {
			op = "==";
		}
		else if (op == "<") {
			op = ">=";
		}
		else if (op == "<=") {
			op = ">";
		}
		else if (op == ">") {
			op = "<=";
		}
		else if (op == ">=") {
			op = "<";
		}

		output_inter(op, "T_" + to_string(res1), "T_" + to_string(res2), "LABEL_IF_" + to_string(if_lable_num));
		if_lable_num+=2;
		/*output_inter(op, "T_" + to_string(res1), "T_" + to_string(res2), "LABEL_IF_" + to_string(if_lable_num));
		if_lable_num++;*/
	}

	return if_lable_num - 2;

}

int if_end_tra(int table_num) {
	output_inter("j","","", "LABEL_IF_" + to_string(table_num+1));
	inter_out << "LABEL_IF_" + to_string(table_num) +":"<< endl;
	return table_num+1;
}
void else_end_tra(int table_num) {
	inter_out << "LABEL_IF_" + to_string(table_num) + ":" << endl;
	return;
}



/*
switch(<���ʽ>)
���ر�����ʽ�����T_n���n
֮�����case��defaultʱT_num������
*/
int switch_begin_tra(vector<string> &sym, vector<string> &values) {
	//ȥ��
	while (sym[W_i] != "SWITCHSYM") {
		W_i++;
	}
	int res1 = 0;
	W_i = W_i + 2;
	res1 = expression_tra(sym, values);
	return res1;
}
/*
�������ǲ��ǵ�һ��case�Լ��ȽϵĶ���T_n���n
�����¸�case�ı�ǩ��
case <����>��
ob��switch��ָ���ıȽ϶���id  T_ob
��switch���õ���T_n  n != ob
*/

int case_tra(bool first,int ob,int table_num,vector<string> &sym, vector<string> &values) {
	if (!first) {
		output_inter("j","","","LABEL_SWITCH_END_"+to_string(switch_end_label_num));
		inter_out << "LABEL_CASE_" + to_string(table_num) + ":" << endl;
	}
	while (sym[W_i] != "CASESYM") {
		W_i++;
	}
	W_i++;
	int res1 = 0;

	if (T_num == ob) {
		T_num++;
	}
	//��ֵ
	if (sym[W_i] == "NUMBER") {
		output_inter("=", "(" + values[W_i] + ")", "", "T_" + to_string(T_num));
		res1 = T_num;
		T_num++;
	}
	else {
		output_inter("=", "[" + values[W_i] + "]", "", "T_" + to_string(T_num));
		res1 = T_num;
		T_num++;
	}
	//�Ƚ�
	output_inter("!=", "T_" + to_string(res1), "T_" + to_string(ob), "LABEL_CASE_" + to_string(case_lable_num));
	case_lable_num++;

	return case_lable_num - 1;
}
void default_tra(int table_num) {
	output_inter("j", "", "", "LABEL_SWITCH_END_" + to_string(switch_end_label_num));
	inter_out << "LABEL_CASE_" + to_string(table_num) + ":" << endl;

}
void switch_end_tra() {
	inter_out << "LABEL_SWITCH_END_" + to_string(switch_end_label_num) + ":" << endl;
	switch_end_label_num++;
}

/*
while(<����>)
*/
int while_begin_tra(vector<string> &sym, vector<string> &values) {
	//ȥ��
	while (sym[W_i] != "WHILESYM") {
		W_i++;
	}
	//ѭ��ʱ����������
	inter_out << "LABEL_WHILE_HEAD_" + to_string(while_lable_num) + ":" << endl;

	int res1 = 0, res2 = 0;
	string op = "";
	W_i = W_i + 2;
	T_num = 0;
	res1 = expression_tra(sym, values);
	if (values[W_i] == ")") {
		//output_inter("=","(0)" , "", "T_" + to_string(T_num));
		//res2 = T_num;
		//T_num++;
		output_inter("==", "T_" + to_string(res1), "(0)", "LABEL_WHILE_" + to_string(while_lable_num));
		while_lable_num++;
	}
	else {
		op = values[W_i];

		W_i++;
		res2 = expression_tra(sym, values);
		//��op
		if (op == "==") {
			op = "!=";
		}
		else if (op == "!=") {
			op = "==";
		}
		else if (op == "<") {
			op = ">=";
		}
		else if (op == "<=") {
			op = ">";
		}
		else if (op == ">") {
			op = "<=";
		}
		else if (op == ">=") {
			op = "<";
		}
		output_inter(op, "T_" + to_string(res1), "T_" + to_string(res2), "LABEL_WHILE_" + to_string(while_lable_num));
		while_lable_num++;
		/*output_inter(op, "T_" + to_string(res1), "T_" + to_string(res2), "LABEL_WHILE_" + to_string(while_lable_num));
		while_lable_num++;*/
	}

	return while_lable_num - 1;
}
void while_end_tra(int table_num) {
	output_inter("j", "", "", "LABEL_WHILE_HEAD_" + to_string(table_num));
	inter_out << "LABEL_WHILE_" + to_string(table_num) + ":" << endl;
}

/*
����ʶ���� (�������ʽ��|��[�������ʽ����]��=�����ʽ��) ;
*/
void assign_tra(vector<string> &sym, vector<string> &values) {
	string ident = values[W_i];
	int res = 0,index = 0;
	W_i += 1;
	if (values[W_i] == "[") {
		W_i++;
		index = expression_tra(sym, values);
		W_i+=2;
		res = expression_tra(sym, values);
		output_inter("=", "T_" + to_string(res), "", ident+"["+"T_"+to_string(index)+"]");
	}
	else {
		W_i++;
		res = expression_tra(sym, values);
		output_inter("=", "T_" + to_string(res), "", ident);
	}
}
/*
scanf ��(������ʶ����{,����ʶ����}��)��
*/
void scanf_tra(vector<string> &sym, vector<string> &values) {
	//ȥ��
	while (sym[W_i] != "SCANFSYM") {
		W_i++;
	}
	W_i+=2;
	int size = sym.size();
	while (W_i < size && sym[W_i] == "IDENT") {
		output_inter("scanf", "", "", values[W_i]);
		W_i += 2;
	}

}
/*
��д��䣾    ::= printf ��(�� ���ַ�����(,�����ʽ�� | ���գ�) | �����ʽ��) ��)��
*/
void printf_tra(vector<string> &sym, vector<string> &values) {
	//ȥ��
	while (sym[W_i] != "PRINTFSYM") {
		W_i++;
	}
	W_i += 2;
	int res1 = 0;
	if (sym[W_i] == "STRING") {
		output_inter("printf","","","{"+values[W_i]+"}");
		if (values[W_i + 1] == ",") {
			W_i += 2;
			res1 = expression_tra(sym, values);
			output_inter("printf", "", "", "T_"+to_string(res1));
		}
	}
	else {
		res1 = expression_tra(sym, values);
		output_inter("printf", "", "", "T_" + to_string(res1));
	}


}
/*
����ʶ������(�������ʽ��{,�����ʽ��}�����գ���)��;
*/
int funcCall_tra(vector<string> &sym, vector<string> &values) {
	while (!(sym[W_i] == "IDENT" && sym[W_i + 1] == "LPAREN")) {
		W_i++;
	}


	int res = 0;
	int i = 0;
	string func_name = values[W_i];
	string temp = "";
	int size = sym.size();
	vector<int> arg_arr;
	W_i += 2;
	//�޲�
	if (values[W_i] == ")") {
		output_inter("=", func_name + "()", "", "T_" + to_string(T_num));
		res = T_num;
		T_num++;
	}
	else {
		do {
			if (values[W_i] == ",") {
				W_i++;
			}
			res = expression_tra(sym, values);
			arg_arr.push_back(res);
		//} while (W_i < size && values[W_i] != ";");
		} while (W_i < size && values[W_i] == ",");
		if (values[W_i] == ")") {
			W_i++;
		}
		temp += func_name+"(";
		while (i < arg_arr.size() - 1) {
			temp += "T_" + to_string(arg_arr[i]) + ",";
			i++;
		}
		temp += "T_" + to_string(arg_arr[i])+")";
		output_inter("=", temp, "", "T_" + to_string(T_num));
		res = T_num;
		T_num++;
	}


	return res;
}

/*
return ;
return (<���ʽ>);
*/
void return_tra(vector<string> &sym, vector<string> &values) {
	//ȥ��
	while (sym[W_i] != "RETURNSYM") {
		W_i++;
	}
	W_i+=2;
	int res1 = 0;
	if (W_i < values.size()) {
		res1 = expression_tra(sym, values);
		output_inter("return", "", "", "T_" + to_string(res1));
	}
	else {
		output_inter("return", "", "", "");
	}
}

/*
����expression_tra��W_i��ָ����ʽ������Ǹ��ַ�
*/

int expression_tra(vector<string> &sym, vector<string> &values) {
	string op = "";
	int res1 = 0, res2 = 0;		//����ֵ����ʱ������� T_res
	if (sym[W_i] == "MINUS" || sym[W_i] == "PLUS") {
		op = values[W_i];
		W_i++;
	}
	res2 = item_tra(sym, values);
	if (op != "") {
		output_inter("=", "(0)","", "T_" + to_string(T_num));
		res1 = T_num;
		T_num++;
		output_inter(op,"T_"+to_string(res1), "T_" + to_string(res2), "T_"+ to_string(T_num));
		res1 = T_num;
		T_num++;
	}
	else {
		res1 = res2;
	}

	while (W_i < sym.size() && (sym[W_i] == "MINUS" || sym[W_i] == "PLUS")) {
		op = values[W_i];
		W_i++;
		res2 = item_tra(sym, values);
		output_inter(op, "T_" + to_string(res1), "T_" + to_string(res2), "T_" + to_string(T_num));
		res1 = T_num;
		T_num++;
	}
	return res1;
}

int item_tra(vector<string> &sym, vector<string> &values) {
	string op = "";
	int res1 = 0, res2 = 0;

	res1 = factor_tra(sym,values);
	while (W_i < sym.size() && (sym[W_i] == "MULT" || sym[W_i] == "DIV")) {
		op = values[W_i];
		W_i++;
		res2 = factor_tra(sym, values);
		output_inter(op, "T_" + to_string(res1), "T_" + to_string(res2), "T_" + to_string(T_num));
		res1 = T_num;
		T_num++;
	}

	return res1;
}

int factor_tra(vector<string> &sym, vector<string> &values) {
	int res1 = 0;
	string ident_name = "";
	if (sym[W_i] == "IDENT") {
		ident_name = values[W_i];
		W_i++;
		//����
		if (sym[W_i] == "LBRACK") {
			W_i++;
			if (sym[W_i] != "RBRACK") {
				res1 = expression_tra(sym, values);
				output_inter("=", ident_name+"["+"T_" + to_string(res1)+"]","", "T_" + to_string(T_num));
				res1 = T_num;
				T_num++;
			}
			if (sym[W_i] == "RBRACK") {
				W_i++;
			}
			else {
				W_i++;
			}
		}
		//��������
		else if (sym[W_i] == "LPAREN") {
			//ȷ��funcCall_tra�ܹ�ʶ�����***********
			W_i -= 1;
			res1 = funcCall_tra(sym,values);
			/*output_inter("=", ident_name + "(" + "T_" + to_string(res1) + ")", "", "T_" + to_string(T_num));
			res1 = T_num;
			T_num++;*/
		}
		//������ʶ��
		else {
			output_inter("=", ident_name , "", "T_" + to_string(T_num));
			res1 = T_num;
			T_num++;
		}
	}
	else if (sym[W_i] == "NUMBER" || sym[W_i] == "CHAR") {

		if (sym[W_i] == "NUMBER") {
			output_inter("=", "("+values[W_i]+")", "", "T_" + to_string(T_num));
		}
		else{
			output_inter("=", "[" + values[W_i] + "]", "", "T_" + to_string(T_num));
		}
		res1 = T_num;
		T_num++;
		W_i++;
	}
	else if (sym[W_i] == "LPAREN") {
		W_i++;
		if (sym[W_i] != "RPAREN") {
			res1 = expression_tra(sym, values);
			output_inter("=", "T_" + to_string(res1), "", "T_" + to_string(T_num));
			res1 = T_num;
			T_num++;
		}
		if (sym[W_i] == "RPAREN") {
			W_i++;
		}
	}

	return res1;
}

void output_inter(string op,string arg1,string arg2,string result) {
	inter_out << op + "," + arg1 + "," + arg2 + "," + result << endl;
}

bool j_main_mark = false;
void JToMain() {
	if (!j_main_mark) {
		j_main_mark = true;
		output_inter("j", "", "", "LABEL_FUNC_main");
	}
}

#endif // INTER_CODE_H_INCLUDED
