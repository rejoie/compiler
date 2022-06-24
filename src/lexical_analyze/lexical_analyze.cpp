#include <cstdio>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
using namespace std;

map<string, int> symbol_map;

string token = "";	// token存储
int type;			// token类型
char ch;			//当前读取的字符
int line_num = 1;	//行号
int colume_num = 1; //列号

void get_next_char();					 //读取下一个字符
bool is_letter(char ch);				 //判断是否为字母
bool is_digit(char ch);					 //判断是否为数字
char getnbc();							 //获得非空格字符
void concat();							 //将当前字符拼接到token中
void retract();							 //回退一个字符
int reserve();							 //返回关键字的类别编码
int symbol();							 //返回标识符的类别编码
int constant();							 //返回常数的类别编码
void print_word(string token, int type); //打印单词
void error(string error);				 //错误处理函数

void lexical_analyze(); //词法分析函数

FILE *fp1 = fopen("../../out/test.dyd", "w");
FILE *fp2 = fopen("../../out/test.err", "w");

int main()
{
	symbol_map["begin"] = 1;
	symbol_map["end"] = 2;
	symbol_map["integer"] = 3;
	symbol_map["if"] = 4;
	symbol_map["then"] = 5;
	symbol_map["else"] = 6;
	symbol_map["function"] = 7;
	symbol_map["read"] = 8;
	symbol_map["write"] = 9;
	symbol_map["symbol"] = 10;
	symbol_map["constant"] = 11;
	symbol_map["="] = 12;
	symbol_map["<>"] = 13;
	symbol_map["<="] = 14;
	symbol_map["<"] = 15;
	symbol_map[">="] = 16;
	symbol_map[">"] = 17;
	symbol_map["-"] = 18;
	symbol_map["*"] = 19;
	symbol_map[":="] = 20;
	symbol_map["("] = 21;
	symbol_map[")"] = 22;
	symbol_map[";"] = 23;
	symbol_map["EOLN"] = 24;
	symbol_map["EOF"] = 25;

	freopen("../../test.pas", "r", stdin);
	if (fp1 == NULL)
	{
		printf("test.dyd open file error\n");
		return 0;
	}
	if (fp2 == NULL)
	{
		printf("test.err open file error\n");
		return 0;
	}
	while (ch != EOF)
	{
		lexical_analyze();
		get_next_char();
		if (ch == '\n')
		{
			line_num++;
			colume_num = 1;
			print_word("EOLN", 24);
		}
		else if (ch == EOF)
		{
			break;
		}
		retract();
	}
	print_word("EOF", 25);
	fclose(fp1);
	fclose(fp2);

	return 0;
}

void get_next_char()
{
	ch = getchar();
	colume_num++;
}

bool is_letter(char ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool is_digit(char ch)
{
	return ch >= '0' && ch <= '9';
}

char getnbc()
{
	while (ch == ' ' || ch == '\t' || ch == '\n' || ch == '\r')
	{
		get_next_char();
		if (ch == '\n')
		{
			line_num++;
			colume_num = 1;
			print_word("EOLN", 24);
		}
	}
	return ch;
}

void concat()
{
	token += ch;
}

void retract()
{
	ungetc(ch, stdin);
	colume_num--;
	ch = ' ';
}

int reserve(string token)
{
	if (symbol_map.find(token) != symbol_map.end())
	{
		return symbol_map[token];
	}
	else
	{
		return 0;
	}
}

int symbol()
{
	return symbol_map["symbol"];
}

int constant()
{
	return symbol_map["constant"];
}

void print_word(string token, int type)
{
	fprintf(fp1, "%16s %2d\n", token.c_str(), type);
}

void error(string error)
{
	printf("***LINE: %3d COLUME: %3d near '%s' %s\n", line_num, colume_num, (token + ch).c_str(), error.c_str());
	fprintf(fp2, "***LINE: %3d COLUME: %3d near '%s' %s\n", line_num, colume_num, (token + ch).c_str(), error.c_str());
}

void lexical_analyze()
{
	token = "";
	get_next_char();
	ch = getnbc();
	switch (ch)
	{
	case 'a':
	case 'b':
	case 'c':
	case 'd':
	case 'e':
	case 'f':
	case 'g':
	case 'h':
	case 'i':
	case 'j':
	case 'k':
	case 'l':
	case 'm':
	case 'n':
	case 'o':
	case 'p':
	case 'q':
	case 'r':
	case 's':
	case 't':
	case 'u':
	case 'v':
	case 'w':
	case 'x':
	case 'y':
	case 'z':
	case 'A':
	case 'B':
	case 'C':
	case 'D':
	case 'E':
	case 'F':
	case 'G':
	case 'H':
	case 'I':
	case 'J':
	case 'K':
	case 'L':
	case 'M':
	case 'N':
	case 'O':
	case 'P':
	case 'Q':
	case 'R':
	case 'S':
	case 'T':
	case 'U':
	case 'V':
	case 'W':
	case 'X':
	case 'Y':
	case 'Z':
		while (is_letter(ch) || is_digit(ch))
		{
			concat();
			get_next_char();
		}
		retract();
		if (token.length() >= 16)
		{
			error("identifier is too long");
		}

		type = reserve(token);
		if (type != 0)
		{
			print_word(token, type);
		}
		else
		{
			type = symbol();
			print_word(token, type);
		}
		break;
	case '0':
	case '1':
	case '2':
	case '3':
	case '4':
	case '5':
	case '6':
	case '7':
	case '8':
	case '9':
		while (is_digit(ch))
		{
			concat();
			get_next_char();
		}
		retract();
		type = constant();
		print_word(token, type);
		break;
	case '=':
	case '-':
	case '*':
	case '(':
	case ')':
	case ';':
		concat();
		print_word(token, reserve(token));
		break;
	case '<':
		ch = getchar();
		if (ch == '=')
		{
			print_word("<=", reserve("<="));
		}
		else if (ch == '>')
		{
			print_word("<>", reserve("<>"));
		}
		else
		{
			retract();
			print_word("<", reserve("<"));
		}
		break;
	case '>':
		get_next_char();
		if (ch == '=')
		{
			print_word(">=", reserve(">="));
		}
		else
		{
			retract();
			print_word(">", reserve(">"));
		}
		break;
	case ':':
		concat();
		get_next_char();
		if (ch == '=')
		{
			print_word(":=", reserve(":="));
		}
		else
		{
			retract();
			error("`:` doesn't match");
		}
		break;
	default:
		if (ch != EOF)
		{
			error("illegal character");
		}
		break;
	}
}
