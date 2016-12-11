#ifndef MIPS_CODE_H_INCLUDED
#define MIPS_CODE_H_INCLUDED

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <vector>
#include "table.h"
/*
由于Mars的特殊性，不管是int还是char都用4个字节来存
运行栈的分配放在这里
*/

//一条中间指令
typedef struct inter_inses {
	string op = "";
	string ob1 = "";
	string ob2 = "";
	string result = "";
} inter_ins;


int pc = 4194304;
int temp_reg_id = 8; //临时用到的寄存器 8~15  翻译完一条中间指令后复位,只用到一次，用完也复位getRegNum记录下调用之前的，返回值用完就复位
string reg_type[32];	//寄存器中数据类型 int/char
int used_reg_num = -1;		//用来标识T_0~T_99在某个函数中有多少被被使用，使用到的在函数调用前要保存，调用完要恢复


ifstream mips_in;
ofstream mips_out;
vector<string> mipsCode;	//存放所有翻译出的指令和标签
inter_ins ins;		//当前读到的指令
int string_id = 0;	//字符串的id
vector<string> used_string; //已经保存的字符串
int func_level = 0;
normal_table now_func_nt;
function_table now_func_ft;

void mips_init();
void store_code(string str);
void mips_func(string name);
bool getIns();
void inter_tra(int level);
void output_mips();
void save_string();
int getRegNum(string name);
int func_call(string str);
int ob_mips_tra(string ob);
void printf_mips_tra();
void scanf_mips_tra();
void return_mips_tra();
int result_mips_tra(int reg1, int reg2);

void store_code(string str) {
	mipsCode.push_back(str);
	if (str[0] != 'T') {
		pc + 4;
	}
}

void mips_init() {
	mips_in.open("interCode.txt");
	remove("mipsCode.txt");
	mips_out.open("mipsCode.txt");

	mips_out << ".data " + to_string(used_data_mem+100)<< endl;
	while (getIns());
	output_mips();
}

/*
获取一条指令，分为四部分
如果读到的是标签就直接输出
*/
bool getIns() {
	int i = 0;
	string op = "", ob1 = "", ob2 = "", result = "";
	string label = "";
	string func_name = "";
	char line[256] ;
	if (!mips_in.eof()) {
		mips_in.getline(line, 256);
		//cout << line << endl;
		if (line[i] == 'L') {
			while (line[i] != '\0') {
				if (line[i] == 'F' && line[i+1] == 'U' && line[i+2] == 'N' && line[i+3] == 'C') {
					label += "FUNC_";
					i += 5;
					while (line[i] != ':') {
						func_name += line[i];
						i++;
					}
					mips_func(func_name);
					label += func_name;
					label += ":";
					store_code(label);
					//存返回地址
					store_code("sw $ra 0($sp)");
					return true;
				}
				label += line[i];
				i++;
			}
			//mips_out << label << endl;
			store_code(label);
		}
		else if(line[i] != '\0'){
			while (line[i] != ',') {
				op += line[i];
				i++;
			}
			i++;
			//可能存在于字符串中的, 和函数调用中的,
			while (line[i] != ',') {
				if (line[i] == '{') {
					while (line[i] != '}') {
						ob1 += line[i];
						i++;
					}
				}
				else if (line[i] == '(') {
					while (line[i] != ')') {
						ob1 += line[i];
						i++;
					}
				}
				ob1 += line[i];
				i++;
			}
			i++;
			//第二个操作数一定是T_n
			while (line[i] != ',') {
				ob2 += line[i];
				i++;
			}
			i++;
			//while (line[i] != '\0' && line[i] != ',') {
			while (line[i] != '\0') {
				result += line[i];
				i++;
			}
			ins.op = op;
			ins.ob1 = ob1;
			ins.ob2 = ob2;
			ins.result = result;
			save_string();
			inter_tra(func_level);
		}
		else {
			return false;
		}
		return true;
	}
	else {
		return false;
	}
}

/*
翻译一条指令,处理后转化为一些简单指令，调用basic_inter_tra。
*/
void inter_tra(int level) {
	string ob1_name = "";
	int i = 0;
	int reg1 = 0, reg2 = 0;
	if (ins.op == "=") {
		temp_reg_id = 8;
		reg1 = ob_mips_tra(ins.ob1);
		result_mips_tra(reg1,0);
	}
	else if(ins.op == ">" || ins.op == ">=" || ins.op == "==" || ins.op == "!=" || ins.op == "<" || ins.op == "<=" ){
		temp_reg_id = 8;
		reg1 = ob_mips_tra(ins.ob1);
		reg2 = ob_mips_tra(ins.ob2);
		result_mips_tra( reg1, reg2);
	}
	else if (ins.op == "+" || ins.op == "-" || ins.op == "*" || ins.op == "/") {
		temp_reg_id = 8;
		reg1 = ob_mips_tra(ins.ob1);
		reg2 = ob_mips_tra(ins.ob2);
		result_mips_tra(reg1, reg2);
	}
	else if (ins.op == "j") {
		temp_reg_id = 8;
		store_code("j "+ ins.result);
	}
	else if (ins.op == "scanf") {
		temp_reg_id = 8;
		scanf_mips_tra();
	}
	else if (ins.op == "printf") {
		temp_reg_id = 8;
		printf_mips_tra();
	}
	else if (ins.op == "return") {
		temp_reg_id = 8;
		return_mips_tra();
	}
	else {
		//error();
		cout << "ERROR : " + ins.op << endl;
	}
}

/*
更新当前函数
在前面加上return语句。防止上一个函数没有return语句
*/
void mips_func(string name) {
	//不是第一个函数
	if (now_func_nt.func_table_id != -1) {
		if (now_func_ft.return_type == "void") {
			store_code("lw $ra 0($sp)");
			store_code("jr $ra");
		}
		else {
			store_code("li $"+to_string(temp_reg_id) + " 0");
			store_code("sw $" + to_string(temp_reg_id) + " 8($sp)");
			store_code("lw $ra 0($sp)");
			store_code("jr $ra");
		}
	}
	now_func_nt = getTableData(0, name,"func");
	now_func_ft = f_table[now_func_nt.func_table_id];
	func_level = now_func_ft.level;
	used_reg_num = -1;

}


/*
printf,,,{"n is too low"}     =>	printf,,,{string_id}
*/
void save_string() {
	string temp = "";
	int i = 0;
	bool mark = false;
	if (ins.result[0] == '{') {

		for (i = 1; i < ins.result.size() - 2; i++) {
			if (ins.result[i] == '\\') {
				temp += "\\";
				temp += "\\";
			}
			else {
				temp += ins.result[i];
			}
		}

		for (i = 0; i < used_string.size(); i++) {
			if (used_string[i] == temp) {
				mark = true;
				break;
			}
		}
		if (mark) {
			ins.result = "{" + to_string(i) + "}";
		}
		else {
			used_string.push_back(temp);
			mips_out << "STRING_" + to_string(string_id) + ": .asciiz " + temp << endl;
			ins.result = "{" + to_string(string_id) + "}";
			string_id++;
		}

	}
	else {

	}
}


void output_mips() {
	int i = 0;
	mips_out << ".text" << endl;
	for (i = 0; i < mipsCode.size(); i++) {
		mips_out << mipsCode[i] << endl;
	}
}

/*
获取寄存器编号，只能用于读，不能写
$29号寄存器作为栈指针，用于运行栈
$8~$15 作为中间运算过程中用的寄存器
$3 返回值
$16~$25作为与 T_n对应的寄存器
*/
int getRegNum(string name) {
	string temp = name.substr(2);
	int addr = used_data_mem + 5000;	//专门用来存中间变量的地址
	int num = atoi(temp.c_str());
	if (used_reg_num < num) {
		used_reg_num = num;
	}
	if (num <= 10) {
		return num + 16;
	}
	else {
		//临时寄存器正在被使用，它里面的值还有用
		if (num - 10 + 8 - 1 < temp_reg_id) {
			//return num - 10 + 8 - 1;
			addr = addr + (num - 10) * 4;
			store_code("lw $" + to_string(temp_reg_id) + " " + to_string(addr) + "($0)");
			temp_reg_id++;
			return temp_reg_id - 1;
		}
		else {
			addr = addr + (num - 10) * 4;
			store_code("lw $" + to_string(temp_reg_id) + " " + to_string(addr) + "($0)");
			temp_reg_id++;
			return temp_reg_id - 1;
		}
	}
}

/*
函数调用
参数：func(arg1,arg2,...)
给出最终保存结果的寄存器编号
用jal跳过去，在函数开始的地方存返回地址
*/
int func_call(string str) {

	int mem_size = 0;		//$sp需要减去的值
	int reg_size = 0;		//只存寄存器16~25的值
	string func_name = "";
	normal_table nt;
	function_table ft;
	int arg_num = 0;
	vector<string> arg_name;
	string temp = "";
	int temp_reg = 0;
	int temp_num = 0;
	int temp_addr = used_data_mem + 1000;
	int i = 0;
	int addr = 12;
	int reg = 0;
	//获取函数名和参数信息
	while (str[i] != '(') {
		func_name += str[i];
		i++;
	}
	i++;
	while (str[i] != ')') {
		if (str[i] == ',') {
			arg_name.push_back(temp);
			temp = "";
			arg_num++;
		}
		else {
			temp+= str[i];
		}
		i++;
	}
	if (temp != "") {
		arg_name.push_back(temp);
		temp = "";
		arg_num++;
	}
	nt = getTableData(0, func_name,"func");
	ft = f_table[nt.func_table_id];
	//先判断参数里面会不会使used_reg_num变大
	for (i = 0; i < arg_name.size(); i++) {
		temp = arg_name[i].substr(2);
		temp_num = atoi(temp.c_str());
		if (temp_num > used_reg_num) {
			used_reg_num = temp_num;
		}
	}
	reg_size = 4 * (used_reg_num + 1);
	//入栈$sp
	store_code("move $"+to_string(temp_reg_id) + " $sp");		//上个$sp的值
	mem_size = 12 + ft.arg_num * 4 + ft.size;
	store_code("subi $sp $sp "+to_string(mem_size+reg_size));
	//存上个$sp的值
	store_code("sw $" + to_string(temp_reg_id) + " 4($sp)");
	//存参数
	for (i = 0; i < arg_name.size(); i++) {
		reg = getRegNum(arg_name[i]);
		store_code("sw $" + to_string(reg) + " " + to_string(addr) + "($sp)");
		addr += 4;

		temp = arg_name[i].substr(2);
		temp_num = atoi(temp.c_str());
		if (temp_num > 10) {
			temp_reg_id--;
		}

	}
	//存保留寄存器
	for (i = 0; i <= used_reg_num; i++) {
		temp = "T_" + to_string(i);
		temp_reg = getRegNum(temp);
		store_code("sw $" + to_string(temp_reg) + " " + to_string(mem_size+i*4) + "($sp)");

		if (i > 10) {
			temp_reg_id--;
		}

	}
	//跳转
	store_code("jal LABEL_FUNC_"+func_name);
	//取保留寄存器的值
	for (i = 0; i <= used_reg_num; i++) {

		if (i <= 10) {
			temp = "T_" + to_string(i);
			temp_reg = getRegNum(temp);
			store_code("lw $" + to_string(temp_reg) + " " + to_string(mem_size + i * 4) + "($sp)");
		}
		else {
			store_code("lw $" + to_string(temp_reg_id) + " " + to_string(mem_size + i * 4) + "($sp)");
			temp_addr = temp_addr + (i - 10) * 4;
			store_code("sw $" + to_string(temp_reg_id) + " " + to_string(temp_addr) + "($0)");
		}


	}
	//返回值
	store_code("lw $3 8($sp)");
	//恢复$sp
	store_code("lw $" + to_string(temp_reg_id) + " 4($sp)");
	store_code("move $sp $" + to_string(temp_reg_id));

	reg_type[3] = ft.return_type;
	return 3;
}

/*
return,,,
return,,T_n
*/
void return_mips_tra() {
	int reg = 0;

	//main()的返回就结束
	if (now_func_nt.name == "main") {
		store_code("li $v0 10");
		store_code("syscall");
		return;
	}

	if (ins.result != "") {
		//存返回值
		reg = getRegNum(ins.result);
		store_code("sw $" + to_string(reg) + " 8($sp)");
	}
	//跳回去
	store_code("lw $ra 0($sp)");
	store_code("jr $ra");
}

/*
对操作数进行翻译
根据输入的字符串翻译成汇编指令，并将最终报存结果的寄存器号返回
记录返回寄存器中数据的类型
*/
int ob_mips_tra(string ob) {
	string ob_name = "";
	string temp = "";
	normal_table nt;
	array_table at;
	function_table ft;
	int i = 0;
	int addr = 0;
	int result = 0;
	int reg = 0;

	if (ob == "") {
		cout <<"null ob" << endl;
	}
	//(234)
	if (ob[0] == '(') {
		if (ob[1] == '+') {
			i = 2;
			while (ob[i] != ')') {
				temp += ob[i];
				i++;
			}
			store_code("li $" + to_string(temp_reg_id) + " " + temp);
		}
		else {
			i = 1;
			while (ob[i] != ')') {
				temp += ob[i];
				i++;
			}
			store_code("li $" + to_string(temp_reg_id) + " " + temp);
		}
		result = temp_reg_id;
		reg_type[result] = "int";
		temp_reg_id++;
	}
	else if (ob[0] == '[') {
		int num = ob[2] - '\0';
		store_code("li $" + to_string(temp_reg_id) + " " + to_string(num));
		result = temp_reg_id;
		reg_type[result] = "char";
		temp_reg_id++;
	}
	else {
		while (i< ob.size() && ob[i] != '(' && ob[i] != '[') {
			ob_name += ob[i];
			i++;
		}
		if (ob[0] == 'T') {
			result = getRegNum(ob);
		}
		//func(T_0,T_1,...)
		else if (i< ob.size() && ob[i] == '(') {
			result = func_call(ob);
		}
		//arr[T_n]
		else if (i< ob.size() && ob[i] == '[') {
			i++;
			while (ob[i] != ']') {
				temp += ob[i];
				i++;
			}
			reg = getRegNum(temp);
			nt = getTableData(func_level, ob_name,"array");
			at = a_table[nt.arr_table_id];
			//局部变量
			if (nt.level != 0) {
				addr = nt.addr_sp;
				//数组下标左移两位  即*4
				store_code("sll $" + to_string(reg) + " $" + to_string(reg) + " 2");
				//加上初始地址
				store_code("addi $" + to_string(reg) + " $" + to_string(reg) + " " + to_string(addr));
				//再加上sp的值
				store_code("add $" + to_string(reg) + " $" + to_string(reg) + " $sp");
				//取值
				store_code("lw $" + to_string(reg) + " 0($" + to_string(reg) + ")");
			}
			//全局变量
			else {
				addr = nt.addr;
				//数组下标左移两位  即*4
				store_code("sll $" + to_string(reg) + " $" + to_string(reg) + " 2");
				//加上初始地址
				store_code("addi $" + to_string(reg) + " $" + to_string(reg) + " " + to_string(addr));
				//取值
				store_code("lw $" + to_string(reg) + " 0($" + to_string(reg) + ")");
			}
			result = reg;
			reg_type[result] = at.etype;

		}
		//ident 变量或者参数
		else {
			nt = getTableData(func_level, ob_name,"");
			//局部变量
			if (nt.level != 0) {
				addr = nt.addr_sp;		//变量的相对地址
				store_code("lw $" + to_string(temp_reg_id) + " " + to_string(addr) + "($sp)");
			}
			//全局变量
			else {
				addr = nt.addr;
				store_code("lw $" + to_string(temp_reg_id) + " " + to_string(addr) + "($0)");
			}
			result = temp_reg_id;
			reg_type[result] = nt.kind;
			temp_reg_id++;
		}

	}
	return result;
}


/*
寄存器中存的数据类型怎么判断？？
*/
void printf_mips_tra() {
	int id = 0;
	int size = ins.result.size();
	int reg = 0;
	if (ins.result[0] == '{') {
		id = atoi(ins.result.substr(1,size-2).c_str());
		store_code("la $a0 STRING_" + to_string(id));
		store_code("li $v0 4");
		store_code("syscall");
	}
	else {
		reg = getRegNum(ins.result);
		if (reg_type[reg] == "char") {
			store_code("move $a0 $"+to_string(reg));
			store_code("li $v0 11");
			store_code("syscall");
		}
		else {
			store_code("move $a0 $" + to_string(reg));
			store_code("li $v0 1");
			store_code("syscall");
		}
	}
}

void scanf_mips_tra() {
	int reg = 0;
	normal_table nt;
	int addr = 0;
	reg = ob_mips_tra(ins.result);
	//读出来
	if (reg_type[reg] == "char") {
		store_code("li $v0 12");
		store_code("syscall");
		store_code("move $" + to_string(reg)+" $v0");
	}
	else {
		store_code("li $v0 5");
		store_code("syscall");
		store_code("move $" + to_string(reg) + " $v0");
	}
	//存进去  只有标识符一种情况
	nt = getTableData(func_level, ins.result,"");
	//局部变量
	if (nt.level != 0) {
		addr = nt.addr_sp;		//变量的相对地址
		store_code("sw $" + to_string(reg) + " " + to_string(addr) + "($sp)");
	}
	//全局变量
	else {
		addr = nt.addr;
		store_code("sw $" + to_string(reg) + " " + to_string(addr) + "($0)");
	}

}



/*
将值保存在结果里
ident   T_n   arr[T_n]
*/
int result_mips_tra(int reg1, int reg2){
	int result = 0;
	int r_reg = 0;	//目的寄存器
	int addr = 0;
	int i = 0;
	int index = 0;
	string temp = "";
	string r_name = "";

	//用于T_n  n>10的寄存器存储问题
	string reg_temp = "";
	int reg_num = 0;
	int reg_addr = used_data_mem + 1000;

	normal_table nt;
	array_table at;
	if (ins.op == "=") {
		//T_n
		if (ins.result[0] == 'T') {
			reg_temp = ins.result.substr(2);
			reg_num = atoi(reg_temp.c_str());
			if (reg_num <= 10) {
				r_reg = getRegNum(ins.result);
				store_code("move $" + to_string(r_reg) + " $" + to_string(reg1));
				reg_type[r_reg] = reg_type[reg1];
			}
			else {
				reg_addr = reg_addr + (reg_num - 10) * 4;
				store_code("sw $" + to_string(reg1) + " " + to_string(reg_addr) + "($0)");
			}
		}
		//ident    arr[T_n]
		else {
			while (i< ins.result.size() && ins.result[i] != '[') {
				r_name += ins.result[i];
				i++;
			}
			if (i < ins.result.size() && ins.result[i] == '[') {
				i++;
				while (ins.result[i] != ']') {
					temp += ins.result[i];
					i++;
				}
				index = getRegNum(temp);
				nt = getTableData(func_level, r_name,"array");
				at = a_table[nt.arr_table_id];
				//局部变量
				if (nt.level != 0) {
					addr = nt.addr_sp;
					//求出地址保存在寄存器$index中
					store_code("sll $" + to_string(index) + " $" + to_string(index) + " 2");
					store_code("addi $" + to_string(index) + " $" + to_string(index) + " " + to_string(addr));
					store_code("add $" + to_string(index) + " $" + to_string(index) + " $sp");
					//存值
					store_code("sw $" + to_string(reg1) + " 0($" + to_string(index) + ")");
				}
				//全局变量
				else {
					addr = nt.addr;
					//求出地址保存在寄存器$index中
					store_code("sll $" + to_string(index) + " $" + to_string(index) + " 2");
					store_code("addi $" + to_string(index) + " $" + to_string(index) + " " + to_string(addr));
					//存值
					store_code("sw $" + to_string(reg1) + " 0($" + to_string(index) + ")");
				}
			}
			else {
				nt = getTableData(func_level, ins.result,"");
				//局部变量
				if (nt.level != 0) {
					addr = nt.addr_sp;		//变量的相对地址
					store_code("sw $" + to_string(reg1) + " " + to_string(addr) + "($sp)");
				}
				//全局变量
				else {
					addr = nt.addr;
					store_code("sw $" + to_string(reg1) + " " + to_string(addr) + "($0)");
				}

			}
		}
	}
	else if (ins.op == ">" || ins.op == ">=" || ins.op == "==" || ins.op == "!=" || ins.op == "<" || ins.op == "<=") {
		if (ins.op == ">") {
			temp += "bgt";
		}
		else if (ins.op == ">=") {
			temp += "bge";
		}
		else if (ins.op == "==") {
			temp += "beq";
		}
		else if (ins.op == "!=") {
			temp += "bne";
		}
		else if (ins.op == "<") {
			temp += "blt";
		}
		else if (ins.op == "<=") {
			temp += "ble";
		}
		temp += " $" + to_string(reg1) + " $" + to_string(reg2) + +" "+ins.result;
		store_code(temp);
	}
	else if (ins.op == "+" || ins.op == "-" || ins.op == "*" || ins.op == "/") {
		//T_n
		if (ins.result[0] == 'T') {
			reg_temp = ins.result.substr(2);
			reg_num = atoi(reg_temp.c_str());
			if (reg_num <= 10) {
				r_reg = getRegNum(ins.result);
				if (ins.op == "+") {
					store_code("add $" + to_string(r_reg) + " $" + to_string(reg1) + " $" + to_string(reg2));
					reg_type[r_reg] = "int";
				}
				else if (ins.op == "-") {
					store_code("sub $" + to_string(r_reg) + " $" + to_string(reg1) + " $" + to_string(reg2));
					reg_type[r_reg] = "int";
				}
				else if (ins.op == "*") {
					store_code("mult $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("mflo $" + to_string(r_reg));
					reg_type[r_reg] = "int";
				}
				else if (ins.op == "/") {
					store_code("div $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("mflo $" + to_string(r_reg));
					reg_type[r_reg] = "int";
				}
			}
			else {
				reg_addr = reg_addr + (reg_num - 10) * 4;
				if (ins.op == "+") {
					store_code("add $" + to_string(temp_reg_id) + " $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("sw $" + to_string(temp_reg_id) + " " + to_string(reg_addr) + "($0)");
				}
				else if (ins.op == "-") {
					store_code("sub $" + to_string(temp_reg_id) + " $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("sw $" + to_string(temp_reg_id) + " " + to_string(reg_addr) + "($0)");
				}
				else if (ins.op == "*") {
					store_code("mult $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("mflo $" + to_string(temp_reg_id));
					store_code("sw $" + to_string(temp_reg_id) + " " + to_string(reg_addr) + "($0)");
				}
				else if (ins.op == "/") {
					store_code("div $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("mflo $" + to_string(temp_reg_id));
					store_code("sw $" + to_string(temp_reg_id) + " " + to_string(reg_addr) + "($0)");
				}
			}

		}
		//ident    arr[T_n]
		else {
			while (i < ins.result.size() && ins.result[i] != '[') {
				r_name += ins.result[i];
				i++;
			}
			if (i < ins.result.size() && ins.result[i] == '[') {
				i++;
				while (ins.result[i] != ']') {
					temp += ins.result[i];
					i++;
				}
				index = getRegNum(temp);
				nt = getTableData(func_level, r_name,"array");
				at = a_table[nt.arr_table_id];
				//局部变量
				if (nt.level != 0) {
					addr = nt.addr_sp;
					//求出地址保存在寄存器$index中
					store_code("sll $" + to_string(index) + " $" + to_string(index) + " 2");
					store_code("addi $" + to_string(index) + " $" + to_string(index) + " " + to_string(addr));
					store_code("add $" + to_string(index) + " $" + to_string(index) + " $sp");
				}
				//全局变量
				else {
					addr = nt.addr;
					//求出地址保存在寄存器$index中
					store_code("sll $" + to_string(index) + " $" + to_string(index) + " 2");
					store_code("addi $" + to_string(index) + " $" + to_string(index) + " " + to_string(addr));
				}

				//运算
				if (ins.op == "+") {
					store_code("add $" + to_string(temp_reg_id) + " $" + to_string(reg1) + " $" + to_string(reg2));
				}
				else if (ins.op == "-") {
					store_code("sub $" + to_string(temp_reg_id) + " $" + to_string(reg1) + " $" + to_string(reg2));
				}
				else if (ins.op == "*") {
					store_code("mult $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("mflo $" + to_string(temp_reg_id));
				}
				else if (ins.op == "/") {
					store_code("div $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("mflo $" + to_string(temp_reg_id));
				}
				//存值
				store_code("sw $" + to_string(temp_reg_id) + " 0($" + to_string(index) + ")");
			}
			else {
				nt = getTableData(func_level, ins.result,"");
				//局部变量
				if (nt.level != 0) {
					addr = nt.addr_sp;		//变量的相对地址
				}
				//全局变量
				else {
					addr = nt.addr;
				}
				//运算
				if (ins.op == "+") {
					store_code("add $" + to_string(temp_reg_id) + " $" + to_string(reg1) + " $" + to_string(reg2));
				}
				else if (ins.op == "-") {
					store_code("sub $" + to_string(temp_reg_id) + " $" + to_string(reg1) + " $" + to_string(reg2));
				}
				else if (ins.op == "*") {
					store_code("mult $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("mflo $" + to_string(temp_reg_id));
				}
				else if (ins.op == "/") {
					store_code("div $" + to_string(reg1) + " $" + to_string(reg2));
					store_code("mflo $" + to_string(temp_reg_id));
				}
				//存值
				//局部变量
				if (nt.level != 0) {
					store_code("sw $" + to_string(temp_reg_id) + " " + to_string(addr) + "($sp)");
				}
				//全局变量
				else {
					store_code("sw $" + to_string(temp_reg_id) + " " + to_string(addr) + "($0)");
				}

			}
		}
	}
	else {
		cout << "error in result_mips_tra :" + ins.op << endl;
	}
	return result;
}

#endif // MIPS_CODE_H_INCLUDED
