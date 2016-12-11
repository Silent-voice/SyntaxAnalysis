#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
using namespace std;

/*
�Ƿ�Ҫ����=�������������Ƿ���ȣ���������������
���ں���߲���Ϊconst?????????
����ֵ�����뺯������ֵ�����Ƿ���ͬ��������
ÿ����void�����ж����и�return��䡣
*/

/*
�ֲ������ĵ�ַ�������$sp��λ�ã��׵�ַ(8+arg_num*4+4)�����������+4��+����Ԫ�ظ���*4
*/
typedef struct normal_tables {
	string name;	//��ʶ��
	string kind;	//����  int char array func
	string type;	//const var func
	int table_id;	//�ڱ��е��±�
	int size;
	int addr;
	int addr_sp;	//�ֲ����������$sp�ĵ�ַ
	int level;		//��Σ���ʶ�������ĺ���
	int arg_id;		//�����һ�������Ļ������ڲ����б��е�λ�ã�0~n
	int arr_table_id = -1;
	int func_table_id = -1;
	/*
	int def_line;	������
	int use_line[100];	������
	*/
} normal_table;

typedef struct array_tables {
	/*
	int addr;	�׵�ַ���ϱ�����
	int size;	�����С
	int dime;	ά��������1ά
	*/
	int max_index;	//�±��Ͻ�
	string etype;	//����Ԫ������
	int esize;		//����Ԫ�ش�С
}array_table;

typedef struct function_tables {
	int  arg_num;
	string arg_type[100];
	string return_type;
	int size;		//�ֲ�������ռ�ڴ��ܴ�С
	int level;
	int father_level = 0;	//���ķ����ԣ���ʵ�����ڸ������һ˵��ֻ��ȫ�ֱ���
}function_table;


int used_data_mem = 268500992;	//���ڱ�ʹ�õ��������ڴ�ջ����ַ

int addr_sp[100];	//�����б��������$sp�Ĵ����ĵ�ַ


int n_num = 0;	//�Ѽ�¼�ı�ʶ������
int a_num = 0;
int f_num = 0;
normal_table n_table[500];
array_table a_table[200];
function_table f_table[100];
int func_localvar_size = 0;
string now_func_name = "";
string now_func_return_type = "";


void table_error(string lineNum, int type, string name);
bool define_check(string lineNum, int level, vector<string> &values);
void add_func_localvar_size();
bool addTable(int level, vector<string> &values);
bool use_check(string lineNum, int level, vector<string> &sym, vector<string> &values);
normal_table getTableData(int level, string name, string type);
string to_string(int n);

string to_string(int n){
    string str = "";
    stringstream stream;
    stream<<n;
    str=stream.str();
    return str;
}


/*�ַ�����������
strcpy_s(s1,s2)	����s2��s1
strcmp(s1,s2)	�Ƚ�
*/

/*
�ڶ���ʱ��ѯ
����ѯ��������Ƿ��������ñ�ʶ��

���ں������壬�������ֻ�ȽϺ�������ȫ�ֱ��������Ƿ񱻶��壬���в����Ƿ�����
*/
bool define_check(string lineNum, int level, vector<string> &values) {
	int i = 0, index = 0,j = 0;
	normal_table nt;
	int size = values.size();
	int arg_num = 0;
	int mark = false;
	vector<string> args;
	if (values[0] == "const") {
		index+=2;
		while (index < size) {
			for (i = 0; i < n_num; i++) {
				nt = n_table[i];
				if (nt.level == level && nt.name == values[index]) {
					mark = true;
					break;
				}
			}
			if (mark) {
				table_error(lineNum,1, values[index]);
				return true;
			}
			mark = false;
			index += 4;
		}
	}
	else if (values[0] == "int" || values[0] == "char") {
		if (values[2] == "(") {
			//�ȽϺ�����
			for (i = 0; i < n_num; i++) {
				nt = n_table[i];
				if (nt.level == 0 && nt.name == values[1]) {
					mark = true;
					break;
				}
			}
			if (mark) {
				table_error(lineNum, 2, values[1]);
				return true;
			}
			mark = false;
			//�Ƚϲ���
			arg_num = (values.size() - 3) / 3;
			for (i = 0; i < arg_num; i++) {
				if (!args.empty()) {
					for (j = 0; j < args.size(); j++) {
						if (args[j] == values[i * 3 + 4]) {
							mark = true;
							break;
						}
					}
				}
				if (mark) {
					table_error(lineNum, 1, values[i * 3 + 4]);
					return true;
				}
				args.push_back(values[i * 3 + 4]);
			}
		}
		else {
			index = 1;	//������
			while (index < size) {

				if (index + 1 < size && values[index + 1] == "[") {
					for (i = 0; i < n_num; i++) {
						nt = n_table[i];
						if (nt.level == level && nt.name == values[index]) {
							mark = true;
							break;
						}
					}
					if (mark) {
						table_error(lineNum, 1, values[index]);
						return true;
					}
					mark = false;
					index += 5;
				}
				else {
					for (i = 0; i < n_num; i++) {
						nt = n_table[i];
						if (nt.level == level && nt.name == values[index]) {
							mark = true;
							break;
						}
					}
					if (mark) {
						table_error(lineNum, 1, values[index]);
						return true;
					}
					mark = false;
					index += 2;
				}
			}
		}
	}
	else if (values[0] == "void") {
		for (i = 0; i < n_num; i++) {
			nt = n_table[i];
			if (nt.level == 0 && nt.name == values[1]) {
				mark = true;
				break;
			}
		}
		if (mark) {
			table_error(lineNum, 2, values[1]);
			return true;
		}
		mark = false;
		//�Ƚϲ���
		arg_num = (values.size() - 3) / 3;
		for (i = 0; i < arg_num; i++) {
			if (!args.empty()) {
				for (j = 0; j < args.size(); j++) {
					if (args[j] == values[i * 3 + 4]) {
						mark = true;
						break;
					}
				}
			}
			if (mark) {
				table_error(lineNum, 1, values[i * 3 + 4]);
				return true;
			}
			args.push_back(values[i * 3 + 4]);
		}

	}
	else {
		return true;
	}

	return false;
}

/*
��ȡ��ʶ����Ӧ�ı���
type = int/char/array/func
*/
normal_table getTableData(int level, string name, string type) {
	normal_table nt;
	int i = 0;
	bool mark = false;
	for (i = 0; i < n_num; i++) {
		nt = n_table[i];
		if (nt.level == level && nt.name == name ) {
			if (type == "") {
				mark = true;
				break;
			}
			else {
				if (nt.kind == type) {
					mark = true;
					break;
				}
			}
		}
	}
	if (!mark) {
		for (i = 0; i < n_num; i++) {
			nt = n_table[i];
			if (nt.level == 0 && nt.name == name) {
				if (type == "") {
					mark = true;
					break;
				}
				else {
					if (nt.kind == type) {
						mark = true;
						break;
					}
				}
			}
		}
	}
	if (!mark) {
		table_error("-1", 3, name);
	}
	return nt;
}


/*
��д�������оֲ������Ĵ�С
*/
void add_func_localvar_size() {
	int i = 0;
	int f_id = -1;
	normal_table nt;
	for (i = 0; i < n_num; i++) {
		nt = n_table[i];
		if (nt.name == now_func_name) {
			f_table[nt.func_table_id].size = func_localvar_size;
			func_localvar_size = 0;
			return;
		}
	}
}

/*
���������±�ʶ��
const int/char ident = int/char{,ident  = int/char};
int/char ident/ident[45]{,ident/ident[45]};

ͳ�ƾֲ�������С,����������

�����������е�Ӧ��Ҳ������ǰ��εı�ʶ��
������level����0���������������ǰ�������level�洢
int/char ident(�����б�)
void ident(�����б�)
void main()
*/
bool addTable(int level, vector<string> &values) {
	int arr_size = 0;
	int arg_num;
	string arg_type[100];
	int i = 0;
	int index = 0;
	int size = values.size();
	if (values[0] == "const") {
		index++;
		if (values[index] == "int") {
			index++;
			while (index < size) {
				n_table[n_num].name = values[index];
				n_table[n_num].kind = "int";
				n_table[n_num].type = "const";
				n_table[n_num].table_id = n_num;
				n_table[n_num].size = 4;
				if (level == 0) {
					n_table[n_num].addr = used_data_mem;
					used_data_mem += 4;
				}
				else {
					n_table[n_num].addr_sp = addr_sp[level];
					addr_sp[level] += 4;
				}
				n_table[n_num].level = level;
				n_table[n_num].arr_table_id = -1;
				n_table[n_num].func_table_id = -1;
				n_num++;
				func_localvar_size += 4;
				index += 4;
			}

		}
		else if (values[index] == "char") {
			index++;
			while (index < size) {
				n_table[n_num].name = values[index];
				n_table[n_num].kind = "char";
				n_table[n_num].type = "const";
				n_table[n_num].table_id = n_num;
				n_table[n_num].size = 1;
				if (level == 0) {
					n_table[n_num].addr = used_data_mem;
					used_data_mem += 4;
				}
				else {
					n_table[n_num].addr_sp = addr_sp[level];
					addr_sp[level] += 4;
				}
				n_table[n_num].level = level;
				n_table[n_num].arr_table_id = -1;
				n_table[n_num].func_table_id = -1;
				n_num++;
				//func_localvar_size += 1;
				func_localvar_size += 4;
				index += 4;
			}
		}
		else {
			return false;
		}

	}
	else if (values[0] == "int" || values[0] == "char") {
		if (values[2] == "(") {
			func_localvar_size = 0;
			arg_num = (values.size() - 3) / 3;

			if (values[0] == "int") {
				n_table[n_num].name = values[1];
				n_table[n_num].kind = "func";
				n_table[n_num].type = "func";
				n_table[n_num].table_id = n_num;
				n_table[n_num].size;
				n_table[n_num].addr;
				n_table[n_num].level = 0;
				n_table[n_num].arr_table_id = -1;
				n_table[n_num].func_table_id = f_num;
				n_num++;

				f_table[f_num].arg_num = arg_num;
				for (i = 0; i < arg_num; i++) {
					f_table[f_num].arg_type[i] = values[i * 3 + 3];

					if (values[i * 3 + 3] == "int") {
						n_table[n_num].name = values[i * 3 + 4];
						n_table[n_num].kind = "int";
						n_table[n_num].type = "var";
						n_table[n_num].table_id = n_num;
						n_table[n_num].size = 4;
						n_table[n_num].arg_id = i;
						n_table[n_num].addr_sp = 12 + i*4;
						n_table[n_num].level = level;
						n_table[n_num].arr_table_id = -1;
						n_table[n_num].func_table_id = -1;
						n_num++;
					}
					else if (values[i * 3 + 3] == "char") {
						n_table[n_num].name = values[i * 3 + 4];
						n_table[n_num].kind = "char";
						n_table[n_num].type = "var";
						n_table[n_num].table_id = n_num;
						n_table[n_num].size = 1;
						n_table[n_num].arg_id = i;
						n_table[n_num].addr_sp = 12 + i * 4;
						n_table[n_num].level = level;
						n_table[n_num].arr_table_id = -1;
						n_table[n_num].func_table_id = -1;
						n_num++;
					}
					else {
						return false;
					}

				}
				f_table[f_num].return_type = "int";
				f_table[f_num].size;
				f_table[f_num].level = level;
				f_table[f_num].father_level = 0;

				now_func_name = values[1];
				now_func_return_type = "int";
				f_num++;

				addr_sp[level] = 8 + arg_num * 4 + 4;

			}
			else if (values[0] == "char") {
				n_table[n_num].name = values[1];
				n_table[n_num].kind = "func";
				n_table[n_num].type = "func";
				n_table[n_num].table_id = n_num;
				n_table[n_num].size;
				n_table[n_num].addr;
				n_table[n_num].level = 0;
				n_table[n_num].arr_table_id = -1;
				n_table[n_num].func_table_id = f_num;
				n_num++;

				f_table[f_num].arg_num = arg_num;
				for (i = 0; i < arg_num; i++) {
					f_table[f_num].arg_type[i] = values[i * 3 + 3];
					if (values[i * 3 + 3] == "int") {
						n_table[n_num].name = values[i * 3 + 4];
						n_table[n_num].kind = "int";
						n_table[n_num].type = "var";
						n_table[n_num].table_id = n_num;
						n_table[n_num].size = 4;
						n_table[n_num].addr;
						n_table[n_num].addr_sp = 12 + i * 4;
						n_table[n_num].level = level;
						n_table[n_num].arr_table_id = -1;
						n_table[n_num].func_table_id = -1;
						n_num++;
					}
					else if (values[i * 3 + 3] == "char") {
						n_table[n_num].name = values[i * 3 + 4];
						n_table[n_num].kind = "char";
						n_table[n_num].type = "var";
						n_table[n_num].table_id = n_num;
						n_table[n_num].size = 1;
						n_table[n_num].addr;
						n_table[n_num].addr_sp = 12 + i * 4;
						n_table[n_num].level = level;
						n_table[n_num].arr_table_id = -1;
						n_table[n_num].func_table_id = -1;
						n_num++;
					}
					else {
						return false;
					}
				}
				f_table[f_num].return_type = "char";
				f_table[f_num].size;
				f_table[f_num].level = level;
				f_table[f_num].father_level = 0;

				now_func_name = values[1];
				now_func_return_type = "char";
				f_num++;

				addr_sp[level] = 8 + arg_num * 4 + 4;

			}
			else {
				return false;
			}
		}
		else {
			index++;
			while (index < size) {
				if (index + 1 < size && values[index + 1] == "[") {
					arr_size = atoi(values[index+2].c_str());
					if (values[0] == "int") {
						n_table[n_num].name = values[index];
						n_table[n_num].kind = "array";
						n_table[n_num].type = "var";
						n_table[n_num].table_id = n_num;
						n_table[n_num].size = 4 * arr_size;
						if (level == 0) {
							n_table[n_num].addr = used_data_mem;
							used_data_mem += 4 * arr_size;
						}
						else {
							n_table[n_num].addr_sp = addr_sp[level];
							addr_sp[level] += 4 * arr_size;
						}
						n_table[n_num].level = level;
						n_table[n_num].arr_table_id = a_num;
						n_table[n_num].func_table_id = -1;

						a_table[a_num].max_index = arr_size - 1;	//�±��Ͻ�
						a_table[a_num].etype = "int";	//����Ԫ������
						a_table[a_num].esize = 4;

						a_num++;
						n_num++;

						func_localvar_size += 4 * arr_size;
					}
					else if (values[0] == "char") {
						n_table[n_num].name = values[index];
						n_table[n_num].kind = "array";
						n_table[n_num].type = "var";
						n_table[n_num].table_id = n_num;
						n_table[n_num].size = 1 * arr_size;
						if (level == 0) {
							n_table[n_num].addr = used_data_mem;
							used_data_mem += 4 * arr_size;
						}
						else {
							n_table[n_num].addr_sp = addr_sp[level];
							addr_sp[level] += 4 * arr_size;
						}
						n_table[n_num].level = level;
						n_table[n_num].arr_table_id = a_num;
						n_table[n_num].func_table_id = -1;

						a_table[a_num].max_index = arr_size - 1;	//�±��Ͻ�
						a_table[a_num].etype = "char";	//����Ԫ������
						a_table[a_num].esize = 1;

						a_num++;
						n_num++;

						//func_localvar_size += 1 * arr_size;
						func_localvar_size += 4 * arr_size;
					}
					else {
						return false;
					}
					index += 5;
				}
				else {
					if (values[0] == "int") {
						n_table[n_num].name = values[index];
						n_table[n_num].kind = "int";
						n_table[n_num].type = "var";
						n_table[n_num].table_id = n_num;
						n_table[n_num].size = 4;
						if (level == 0) {
							n_table[n_num].addr = used_data_mem;
							used_data_mem += 4;
						}
						else {
							n_table[n_num].addr_sp = addr_sp[level];
							addr_sp[level] += 4;
						}
						n_table[n_num].level = level;
						n_table[n_num].arr_table_id = -1;
						n_table[n_num].func_table_id = -1;
						n_num++;

						func_localvar_size += 4 ;
					}
					else if (values[0] == "char") {
						n_table[n_num].name = values[index];
						n_table[n_num].kind = "char";
						n_table[n_num].type = "var";
						n_table[n_num].table_id = n_num;
						n_table[n_num].size = 1;
						if (level == 0) {
							n_table[n_num].addr = used_data_mem;
							used_data_mem += 4;
						}
						else {
							n_table[n_num].addr_sp = addr_sp[level];
							addr_sp[level] += 4;
						}
						n_table[n_num].level = level;
						n_table[n_num].arr_table_id = -1;
						n_table[n_num].func_table_id = -1;
						n_num++;

						//func_localvar_size += 1;
						func_localvar_size += 4;
					}
					else {
						return false;
					}
					index += 2;
				}
			}
		}
	}
	else if (values[0] == "void") {
		func_localvar_size = 0;
		arg_num = (values.size() - 3) / 3;
		n_table[n_num].name = values[1];
		n_table[n_num].kind = "func";
		n_table[n_num].type = "func";
		n_table[n_num].table_id = n_num;
		n_table[n_num].size;
		n_table[n_num].addr;
		n_table[n_num].level = 0;
		n_table[n_num].arr_table_id = -1;
		n_table[n_num].func_table_id = f_num;
		n_num++;

		f_table[f_num].arg_num = arg_num;
		for (i = 0; i < arg_num; i++) {
			f_table[f_num].arg_type[i] = values[i * 3 + 3];
			if (values[i * 3 + 3] == "int") {
				n_table[n_num].name = values[i * 3 + 4];
				n_table[n_num].kind = "int";
				n_table[n_num].type = "var";
				n_table[n_num].table_id = n_num;
				n_table[n_num].size = 4;
				n_table[n_num].arg_id = i;
				n_table[n_num].addr_sp = 12 + i * 4;
				n_table[n_num].level = level;
				n_table[n_num].arr_table_id = -1;
				n_table[n_num].func_table_id = -1;
				n_num++;
			}
			else if (values[i * 3 + 3] == "char") {
				n_table[n_num].name = values[i * 3 + 4];
				n_table[n_num].kind = "char";
				n_table[n_num].type = "var";
				n_table[n_num].table_id = n_num;
				n_table[n_num].size = 1;
				n_table[n_num].arg_id = i;
				n_table[n_num].addr_sp = 12 + i * 4;
				n_table[n_num].level = level;
				n_table[n_num].arr_table_id = -1;
				n_table[n_num].func_table_id = -1;
				n_num++;
			}
			else {
				return false;
			}
		}
		f_table[f_num].return_type = "void";
		f_table[f_num].size;
		f_table[f_num].level = level;
		f_table[f_num].father_level = 0;

		now_func_name = values[1];
		now_func_return_type = "void";
		f_num++;
		addr_sp[level] = 8 + arg_num * 4 + 4;

	}
	else {
		return false;
	}

	return true;
}

/*
��ʹ��ʱ��ѯ,���ͷ�����false;
����ѯ��ʶ���Ƿ��ڱ���λ򸸲���ж���
�������û�Ҫ�Ƚ�һ�²������ͺ͸���

return ����жϷ���ֵ����

��ֵ����ж�����,���ж��Ҳ��Ǳ��ʽ�����
*/
string left_type = "";
string right_type = "";
bool use_check(string lineNum, int level, vector<string> &sym, vector<string> &values) {
	function_table ft;
	int i = 0, j = 0, arg_i = 0,t_i = 0;
	normal_table nt, temp_nt;
	array_table at, temp_at;
	function_table temp_ft;
	int size = sym.size();
	bool mark = false;		//�Ƿ��ҵ�

	for (i = 0; i < size; i++) {
		//�жϱ�ʶ���Ƿ��ѱ�����
		if (sym[i] == "IDENT") {
			mark = false;
			//�ڷ��ű��в���
			for (t_i = 0; t_i < n_num; t_i++) {
				nt = n_table[t_i];
				if (nt.level == level && nt.name == values[i]) {
					mark = true;
					break;
				}
			}
			if (!mark) {
				for (t_i = 0; t_i < n_num; t_i++) {
					nt = n_table[t_i];
					if (nt.level == 0 && nt.name == values[i]) {
						mark = true;
						break;
					}
				}
			}
			//�ж�
			if (mark) {
				//������Ҫ����һ�����Ĳ������������
				if (nt.type == "func") {
					ft = f_table[nt.func_table_id];
					for (arg_i = 0; arg_i < ft.arg_num; arg_i++) {
						i++;
						while (i < size && sym[i] != "IDENT" && sym[i] != "NUMBER" && sym[i] != "CHAR") {
							i++;
						}
						//����һ����ʶ�����Ƚ�
						if (i < size && sym[i] == "IDENT") {
							mark = false;
							//�ڷ��ű��в���
							for (t_i = 0; t_i < n_num; t_i++) {
								temp_nt = n_table[t_i];
								if (temp_nt.level == level && temp_nt.name == values[i]) {
									mark = true;
									break;
								}
							}
							if (!mark) {
								for (t_i = 0; t_i < n_num; t_i++) {
									temp_nt = n_table[t_i];
									if (temp_nt.level == 0 && temp_nt.name == values[i]) {
										mark = true;
										break;
									}
								}
							}
							//�Ƚ�����
							if (mark) {
								if (temp_nt.kind == "array") {
									temp_at = a_table[temp_nt.arr_table_id];
									if (temp_at.etype == ft.arg_type[arg_i]) {
										continue;
									}
									else {
										table_error(lineNum, 4, nt.name);
										return false;
									}
								}
								else if (temp_nt.kind == "func") {
									temp_ft = f_table[temp_nt.func_table_id];
									if (temp_ft.return_type == ft.arg_type[arg_i]) {
										continue;
									}
									else {
										table_error(lineNum, 4, nt.name);
										return false;
									}
								}
								else{
									if (temp_nt.kind == ft.arg_type[arg_i]) {
										continue;
									}
									else {
										table_error(lineNum, 4, nt.name);
										return false;
									}
								}
							}
							else {
								table_error(lineNum, 3, values[i]);
								return false;
							}
						}
						//����һ������int
						else if (i < size && sym[i] == "NUMBER") {
							if (ft.arg_type[arg_i] == "int") {
								continue;
							}
							else {
								table_error(lineNum, 4, nt.name);
								return false;
							}
						}
						//����һ���ַ�char
						else if (i < size && sym[i] == "CHAR") {
							if (ft.arg_type[arg_i] == "char") {
								continue;
							}
							else {
								table_error(lineNum, 4, nt.name);
								return false;
							}
						}
						else {
							table_error(lineNum,4,nt.name);
							return false;
						}
					}
				}
				//�Ǻ�����ֱ�ӷ��أ���Ҫ�ж��Ժ��ټ�
				else {
					mark = false;
					continue;
				}
			}
			else {
				table_error(lineNum,3,values[i]);
				return false;
			}
		}
		//�жϷ���ֵ�����Ƿ�ƥ��
		else if (sym[i] == "RETURNSYM") {
			if (now_func_return_type == "void") {
				if (sym[i + 1] == "LPAREN" && sym[i + 2] != "RPAREN") {
					table_error(lineNum,5,"");
				}
			}
			else if (now_func_return_type == "int") {
				if (sym[i + 1] == "LPAREN" && sym[i + 2] == "CHAR" && sym[i + 3] == "RPAREN") {
					table_error(lineNum, 5, "");
				}
				else if (sym[i + 1] == "LPAREN" && sym[i + 2] == "IDENT" && sym[i + 3] == "RPAREN") {
					mark = false;
					//�ڷ��ű��в���
					for (t_i = 0; t_i < n_num; t_i++) {
						nt = n_table[t_i];
						if (nt.level == level && nt.name == values[i+2]) {
							mark = true;
							break;
						}
					}
					if (!mark) {
						for (t_i = 0; t_i < n_num; t_i++) {
							nt = n_table[t_i];
							if (nt.level == 0 && nt.name == values[i+2]) {
								mark = true;
								break;
							}
						}
					}
					if (mark) {
						if (nt.kind != now_func_return_type) {
							table_error(lineNum, 5, "");
						}
					}
				}
			}
			else if (now_func_return_type == "char") {
				if (sym[i + 1] == "LPAREN" && sym[i + 2] == "NUMBER" && sym[i + 3] == "RPAREN") {
					table_error(lineNum, 5, "");
				}
				else if (sym[i + 1] == "LPAREN" && sym[i + 2] == "IDENT" && sym[i + 3] == "RPAREN") {
					mark = false;
					//�ڷ��ű��в���
					for (t_i = 0; t_i < n_num; t_i++) {
						nt = n_table[t_i];
						if (nt.level == level && nt.name == values[i+2]) {
							mark = true;
							break;
						}
					}
					if (!mark) {
						for (t_i = 0; t_i < n_num; t_i++) {
							nt = n_table[t_i];
							if (nt.level == 0 && nt.name == values[i+2]) {
								mark = true;
								break;
							}
						}
					}
					if (mark) {
						if (nt.kind != now_func_return_type) {
							table_error(lineNum, 5, "");
						}
					}
				}
			}

		}
		//�жϸ�ֵ�������ƥ������
		else if (sym[i] == "EQL") {
			// ident = ..
			if (i > 1 && sym[i - 1] == "IDENT") {
				mark = false;
				//�ڷ��ű��в���
				for (t_i = 0; t_i < n_num; t_i++) {
					nt = n_table[t_i];
					if (nt.level == level && nt.name == values[i-1]) {
						mark = true;
						break;
					}
				}
				if (!mark) {
					for (t_i = 0; t_i < n_num; t_i++) {
						nt = n_table[t_i];
						if (nt.level == 0 && nt.name == values[i-1]) {
							mark = true;
							break;
						}
					}
				}
				if (mark) {
					left_type = nt.kind;
				}
			}
			// arr[n] = ...
			else {
				j = 1;
				while (i - j >= 0 && sym[i - j] != "LBRACK") {
					j++;
				}
				if (i - j >= 0 && sym[i - j] == "LBRACK") {
					j++;
					if (i - j >= 0 && sym[i - j] == "IDENT") {
						mark = false;
						//�ڷ��ű��в���
						for (t_i = 0; t_i < n_num; t_i++) {
							nt = n_table[t_i];
							if (nt.level == level && nt.name == values[i-j]) {
								mark = true;
								break;
							}
						}
						if (!mark) {
							for (t_i = 0; t_i < n_num; t_i++) {
								nt = n_table[t_i];
								if (nt.level == 0 && nt.name == values[i-j]) {
									mark = true;
									break;
								}
							}
						}
						if (mark) {
							if (nt.arr_table_id != -1) {
								at = a_table[nt.arr_table_id];
								left_type = at.etype;
							}
						}
					}
				}
			}
			//�ж��Ҳ�����
			//int
			if (i+2 < size && sym[i + 1] == "NUMBER" && sym[i + 2] == "SEMICOLON") {
				right_type = "int";
			}
			//char
			else if (i + 2 < size && sym[i + 1] == "CHAR" && sym[i + 2] == "SEMICOLON") {
				right_type = "char";
			}
			//ident
			else if (i + 2 < size && sym[i + 1] == "IDENT" && sym[i + 2] == "SEMICOLON") {
				mark = false;
				//�ڷ��ű��в���
				for (t_i = 0; t_i < n_num; t_i++) {
					nt = n_table[t_i];
					if (nt.level == level && nt.name == values[i+1]) {
						mark = true;
						break;
					}
				}
				if (!mark) {
					for (t_i = 0; t_i < n_num; t_i++) {
						nt = n_table[t_i];
						if (nt.level == 0 && nt.name == values[i+1]) {
							mark = true;
							break;
						}
					}
				}
				if (mark) {
					right_type =  nt.kind;
				}
			}
			//arr[n]
			else if (i + 2 < size && sym[i + 1] == "IDENT" && sym[i + 2] == "LBRACK") {
				j = 3;
				while (i + j < size && sym[i+j] != "RBRACK") {
					j++;
				}
				if (i + j < size && sym[i + j] == "RBRACK") {
					j++;
					if (i + j < size && sym[i + j] == "SEMICOLON") {
						mark = false;
						//�ڷ��ű��в���
						for (t_i = 0; t_i < n_num; t_i++) {
							nt = n_table[t_i];
							if (nt.level == level && nt.name == values[i+j]) {
								mark = true;
								break;
							}
						}
						if (!mark) {
							for (t_i = 0; t_i < n_num; t_i++) {
								nt = n_table[t_i];
								if (nt.level == 0 && nt.name == values[i+j]) {
									mark = true;
									break;
								}
							}
						}
						if (mark) {
							if (nt.arr_table_id != -1) {
								at = a_table[nt.arr_table_id];
								right_type = at.etype;
							}
						}
					}
				}
			}

			if (left_type != "" && right_type != "" && left_type != right_type) {
				table_error(lineNum, 6, "");
			}
			left_type = "";
			right_type = "";
		}
	}

	return false;
}




void table_error(string lineNum, int type, string name) {
	switch (type) {
		case 1:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "����" + name + "�ѱ�����" << endl;
			break;
		case 2:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "����" + name + "�ѱ�����" << endl;
			break;
		case 3:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "��ʶ��" + name + "δ������" << endl;
			break;
		case 4:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "����" + name + "�ĵ��ò������Ϸ�" << endl;
			break;
		case 5:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "����ֵ���Ͳ���" << endl;
			break;
		case 6:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "��ֵ��������������Ͳ���" << endl;
			break;
		default:
			break;
	}
}

#endif // TABLE_H_INCLUDED
