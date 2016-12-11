#include "LexAnalysis.h"


void LexAnalysis::ToUpperString(string &str)
{
	transform(str.begin(), str.end(), str.begin(), (int(*)(int))toupper);
}





void LexAnalysis::Lex_init(char *str)
{
	char *filepath = str;
	in.open(filepath);
	remove("out.txt");
	out.open("out.txt");
	out << "line: ";
	out << lineNum << endl;
	_analysis();
	return;
}

void LexAnalysis::_analysis() {
	_getchar();
	while (ch != EOF) {
		if (ch == ' ' || ch == '\n' || ch == '\t') {
			if (ch == '\n') {
				lineNum++;
				out << "line: ";
				out << lineNum << endl;
			}
			_getchar();
			continue;
		}
		if (ch == '\'') {
			word.push_back(ch);
			_char();
		}
		else if (ch == '\"') {
			word.push_back(ch);
			_string();
		}
		else if ((ch >= '0' && ch <= '9') || ch == '-' || ch == '+') {
			word.push_back(ch);
			_number();
		}
		else if (_letter(ch)) {
			word.push_back(ch);
			_ident();
		}
		else if (_ifSepa(ch)) {
			word.push_back(ch);
			_separator();
		}
		else {
			word.push_back(ch);
			_error(0);
		}
	}
}
void LexAnalysis::_getchar() {
	ch = in.get();
	return;
}
//标识符  在识别的时候需要先判断是不是保留字
void LexAnalysis::_ident() {
	_getchar();
	while (ch != EOF) {
		if ((ch >= '0' && ch <= '9') || _letter(ch)) {
			word.push_back(ch);
			_getchar();
		}
		else if (ch == ' ' || ch == '\n' || ch == '\t' ||
			_ifSepa(ch)) {



			string str = _getword();
			if (!_ifSaved(str)) {
				_output("IDENT", str);
			}
			if (ch == '\n') {
				lineNum++;
				out << "line: ";
				out << lineNum << endl;
				_getchar();
			}
			word.clear();
			break;
		}
		else {
			word.push_back(ch);
			_error(0);
			break;
		}
	}
	return;
}
//字符 'sd'
void LexAnalysis::_char() {
	_getchar();
	if (ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
		(ch >= '0' && ch <= '9') ||
		_letter(ch)) {
		word.push_back(ch);
		_getchar();
	}
	else if (ch == '\'') {
		word.push_back(ch);
		_error(0);
		return;
	}
	else {
		word.push_back(ch);
		_error(0);
		return;
	}

	if (ch == '\'') {
		word.push_back(ch);

		string str = _getword();
		_output("CHAR", str);
		word.clear();
		_getchar();
	}
	else {
		word.push_back(ch);
		_error(0);
	}
	return;
}

//字符串 "asd"
void LexAnalysis::_string() {
	_getchar();
	while (ch == ' ' || ch == '!' || (ch >= '#' && ch <= '~')) {
		word.push_back(ch);
		_getchar();
	}
	if (ch == '\"') {
		word.push_back(ch);

		string str = _getword();
		_output("STRING", str);
		word.clear();
		_getchar();

	}
	else {
		word.push_back(ch);
		_error(0);
	}
	return;
}

//整数
void LexAnalysis::_number() {
	char f_c = ch;
	if (ch == '+' || ch == '-') {
		_getchar();
		if (ch >= '0' && ch <= '9') {
			//是运算符,表达式中会有+0的情况，可能会出现 3 --0，这样我会识别两个减号
			if (sym == "CHAR" || sym == "NUMBER" || sym == "RPAREN" || sym == "IDENT" || ch == '0') {
				if (f_c == '+') {
					_output("PLUS", "+");
				}
				else {
					_output("MINUS", "-");
				}
				word.clear();
			}
			//是整数的一部分
			else {
				word.push_back(ch);
				_number();
			}
		}
		else {
			if (f_c == '+') {
				_output("PLUS", "+");
			}
			else {
				_output("MINUS", "-");
			}
			word.clear();
		}
		return;
	}

	if (ch == '0') {
		_getchar();
		if (ch >= '0' && ch <= '9') {
			word.push_back(ch);
			_error(0);
		}
		else {
			_output("NUMBER", "0");
			word.clear();
		}
		return;
	}

	_getchar();
	while (ch != EOF) {
		if (ch == ' ' || ch == '\n' || ch == '\t' ||
			_ifSepa(ch)) {



			string str = _getword();
			_output("NUMBER", str);

			if (ch == '\n') {
				lineNum++;
				out << "line: ";
				out << lineNum << endl;
				_getchar();
			}

			word.clear();
			break;
		}
		else if (ch >= '0' && ch <= '9') {
			word.push_back(ch);
			_getchar();
		}
		else {
			word.push_back(ch);
			_error(0);
			break;
		}
	}

	return;
}

bool LexAnalysis::_letter(char c) {
	if ((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || c == '_') {
		return true;
	}
	return false;
}

//分隔符
void LexAnalysis::_separator() {

	if (ch == ',') {
		_output("COMMA", ",");
	}
	else if (ch == ';') {
		_output("SEMICOLON", ";");
	}
	else if (ch == ':') {
		_output("COLON", ":");
	}
	else if (ch == '*') {
		_output("MULT", "*");
	}
	else if (ch == '/') {
		_output("DIV", "/");
	}
	else if (ch == '(') {
		_output("LPAREN", "(");
	}
	else if (ch == ')') {
		_output("RPAREN", ")");
	}
	else if (ch == '{') {
		_output("LBRACE", "{");
	}
	else if (ch == '}') {
		_output("RBRACE", "}");
	}
	else if (ch == '[') {
		_output("LBRACK", "[");
	}
	else if (ch == ']') {
		_output("RBRACK", "]");
	}
	else if (ch == '=') {
		_getchar();
		if (ch == '=') {
			_output("DEQL", "==");
		}
		else {
			_output("EQL", "=");
			word.clear();
			return;
		}

	}
	else if (ch == '!') {
		_getchar();
		if (ch == '=') {
			_output("NEQ", "!=");
		}
		else {
			word.push_back(ch);
			_error(0);
			return;
		}
	}
	else if (ch == '<') {
		_getchar();
		if (ch == '=') {
			_output("LEQ", "<=");
		}
		else {
			_output("LSS", "<");
			word.clear();
			return;
		}
	}
	else if (ch == '>') {
		_getchar();
		if (ch == '=') {
			_output("GEQ", ">=");
		}
		else {
			_output("GTR", ">");
			word.clear();
			return;
		}
	}
	else {
		_error(0);
	}
	word.clear();
	_getchar();
	return;
}

void LexAnalysis::_error(int t) {
	string str = "";
	int i = 0;
	for (i = 0; i < word.size(); i++) {
		str += word[i];
	}
	word.clear();
	out << "ERROR " + str << endl;
	_getchar();
	return;
}

//引号算不算分隔符呢
bool LexAnalysis::_ifSepa(char c) {
	if (ch == ',' || ch == ';' || ch == ':' ||
		ch == '+' || ch == '-' || ch == '*' || ch == '/' ||
		ch == '=' || ch == '!' || ch == '<' || ch == '>' ||
		ch == '(' || ch == ')' || ch == '{' || ch == '}' || ch == '[' || ch == ']') {

		return true;
	}
	return false;
}

bool LexAnalysis::_ifSaved(string str) {
	if (str == "const" || str == "int" || str == "char" || str == "void" ||
		str == "if" || str == "else" || str == "while" || str == "switch" || str == "case" || str == "default" ||
		str == "scanf" || str == "printf" || str == "return" || str == "main") {

		_output("saved", str);
	}
	else {
		return false;
	}

	return true;
}

void LexAnalysis::_output(string s, string str) {
	if (s == "saved") {
		string _sym = str;
		ToUpperString(_sym);
		_sym += "SYM";
		sym = _sym;
		out << sym + " " + str << endl;
	}
	else {
		sym = s;
		out << s + " " + str << endl;
	}
}

string LexAnalysis::_getword() {
	string str = "";
	int i = 0;
	//非标识符
	if (word[0] == '\'' || word[0] == '\"') {
		for (i = 0; i < word.size(); i++) {
			str += word[i];
		}
	}
	//标识符不区分大小写
	else {
		for (i = 0; i < word.size(); i++) {
			if (word[i] >= 'A' && word[i] <= 'Z') {
				word[i] = word[i] + 32;
			}
			str += word[i];
		}
	}

	return str;
}
