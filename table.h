#ifndef TABLE_H_INCLUDED
#define TABLE_H_INCLUDED

#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <sstream>
using namespace std;

/*
是否要考虑=左右两侧类型是否相等？？？？？？？？
等于号左边不能为const?????????
返回值类型与函数返回值类型是否相同？？？？
每个非void函数中都得有个return语句。
*/

/*
局部变量的地址填相对于$sp的位置，首地址(8+arg_num*4+4)，后面的依次+4或+数组元素个数*4
*/
typedef struct normal_tables {
	string name;	//标识符
	string kind;	//类型  int char array func
	string type;	//const var func
	int table_id;	//在表中的下标
	int size;
	int addr;
	int addr_sp;	//局部变量相对于$sp的地址
	int level;		//层次，标识符所属的函数
	int arg_id;		//如果是一个参数的话，他在参数列表中的位置，0~n
	int arr_table_id = -1;
	int func_table_id = -1;
	/*
	int def_line;	声明行
	int use_line[100];	引用行
	*/
} normal_table;

typedef struct array_tables {
	/*
	int addr;	首地址，上表已有
	int size;	数组大小
	int dime;	维数，都是1维
	*/
	int max_index;	//下标上界
	string etype;	//数组元素类型
	int esize;		//数组元素大小
}array_table;

typedef struct function_tables {
	int  arg_num;
	string arg_type[100];
	string return_type;
	int size;		//局部变量所占内存总大小
	int level;
	int father_level = 0;	//就文法而言，其实不存在父层次这一说，只有全局变量
}function_table;


int used_data_mem = 268500992;	//正在被使用的数据区内存栈顶地址

int addr_sp[100];	//函数中变量相对于$sp寄存器的地址


int n_num = 0;	//已记录的标识符个数
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


/*字符串操作函数
strcpy_s(s1,s2)	复制s2到s1
strcmp(s1,s2)	比较
*/

/*
在定义时查询
仅查询本层次中是否已声明该标识符

对于函数定义，这里可以只比较函数名在全局变量区中是否被定义，还有参数是否重名
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
			//比较函数名
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
			//比较参数
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
			index = 1;	//变量名
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
		//比较参数
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
获取标识符对应的表项
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
填写函数表中局部变量的大小
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
向表中添加新标识符
const int/char ident = int/char{,ident  = int/char};
int/char ident/ident[45]{,ident/ident[45]};

统计局部变量大小,不包含参数

函数参数表中的应该也算做当前层次的标识符
函数的level都按0处理，函数参数还是按正常的level存储
int/char ident(参数列表)
void ident(参数列表)
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

						a_table[a_num].max_index = arr_size - 1;	//下标上界
						a_table[a_num].etype = "int";	//数组元素类型
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

						a_table[a_num].max_index = arr_size - 1;	//下标上界
						a_table[a_num].etype = "char";	//数组元素类型
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
在使用时查询,不和法返回false;
仅查询标识符是否在本层次或父层次中定义
函数调用还要比较一下参数类型和个数

return 语句判断返回值类型

赋值语句判断类型,不判断右侧是表达式的情况
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
	bool mark = false;		//是否被找到

	for (i = 0; i < size; i++) {
		//判断标识符是否已被声明
		if (sym[i] == "IDENT") {
			mark = false;
			//在符号表中查找
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
			//判断
			if (mark) {
				//函数还要考虑一下他的参数类型与个数
				if (nt.type == "func") {
					ft = f_table[nt.func_table_id];
					for (arg_i = 0; arg_i < ft.arg_num; arg_i++) {
						i++;
						while (i < size && sym[i] != "IDENT" && sym[i] != "NUMBER" && sym[i] != "CHAR") {
							i++;
						}
						//遇到一个标识符，比较
						if (i < size && sym[i] == "IDENT") {
							mark = false;
							//在符号表中查找
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
							//比较类型
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
						//遇到一个整数int
						else if (i < size && sym[i] == "NUMBER") {
							if (ft.arg_type[arg_i] == "int") {
								continue;
							}
							else {
								table_error(lineNum, 4, nt.name);
								return false;
							}
						}
						//遇到一个字符char
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
				//非函数先直接返回，需要判断以后再加
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
		//判断返回值类型是否匹配
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
					//在符号表中查找
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
					//在符号表中查找
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
		//判断赋值语句类型匹配问题
		else if (sym[i] == "EQL") {
			// ident = ..
			if (i > 1 && sym[i - 1] == "IDENT") {
				mark = false;
				//在符号表中查找
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
						//在符号表中查找
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
			//判断右侧类型
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
				//在符号表中查找
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
						//在符号表中查找
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
			cout << "变量" + name + "已被定义" << endl;
			break;
		case 2:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "函数" + name + "已被定义" << endl;
			break;
		case 3:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "标识符" + name + "未被定义" << endl;
			break;
		case 4:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "函数" + name + "的调用参数不合法" << endl;
			break;
		case 5:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "返回值类型不符" << endl;
			break;
		case 6:
			cout << "Line: ";
			cout << lineNum;
			cout << " Error: ";
			cout << "赋值语句左右两侧类型不符" << endl;
			break;
		default:
			break;
	}
}

#endif // TABLE_H_INCLUDED
