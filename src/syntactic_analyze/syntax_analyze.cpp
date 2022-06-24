#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>
#include <fstream>
#include <map>
#include <vector>
using namespace std;

map<string, int> symbol_map;

typedef enum
{
	integer
} types; //支持类型的集合

typedef struct var
{				  //变量名表
	string vname; //变量名
	string vproc; //变量所在过程
	bool vkind;	  //变量种类
	types vtype;  //变量类型
	int vlev;	  //变量层次
	int vadr;	  //变量在变量表中的位置
} var;

typedef struct proc
{				  //过程名表
	string pname; //过程名
	types vtype;  //过程类型
	int plev;	  //过程层次
	int fadr;	  //第一个变量在变量表中的位置
	int ladr;	  //最后一个变量在变量表中的位置
} proc;
vector<var> variable_table;		  //变量表
vector<var> tmp_variable_table;	  //临时变量表
vector<proc> procedure_table;	  //过程表
vector<proc> tmp_procedure_table; //临时过程表

string token = "";				   // token存储
int type;						   // token类型
char ch[20];					   //临时字符数组
int line_num = 1;				   //行号
string old_token = "";			   //上一个token
int old_type = 0;				   //上一个token类型
bool retract_flag = false;		   //回退标志
int p_token = 0;				   // token指针
string current_proc_name = "main"; //当前过程名
int level = 0;					   //层次
vector<proc> procedure_stack;	   //过程栈
string keyword[] = {"begin", "end", "integer", "if", "then", "else", "function", "read", "write"};

var v_tmp;	//临时变量
proc p_tmp; //临时过程

void match_token(string t); //匹配token
void get_next_token();		//读取下一个token
bool is_letter(char ch);	//判断是否为字母
bool is_digit(char ch);		//判断是否为数字
void retract();				//回退整个单词和种别

void assign_variable(var v, string vname, string vproc, bool vkind, types vtype, int vlev, int vadr); //赋值变量
void assign_procedure(proc p, string pname, types vtype, int plev, int fadr, int ladr);				  //获取下一个token
void add_variable_table(string vname, string vproc, bool vkind, types vtype, int vlev);				  //添加变量
void add_tmp_variable_table(string vname, string vproc, bool vkind, types vtype, int vlev);			  //添加临时变量
void add_procedure_table(string pname, types vtype, int plev, int fadr, int ladr);					  //添加过程
void add_tmp_procedure_table(string pname, types vtype, int plev, int fadr, int ladr);				  //添加临时过程
bool variable_existed(string vname, string vproc, bool vkind, types vtype, int vlev);				  //检查变量是否存在
bool find_tmp_variable(string vname, string vproc, bool vkind, types vtype, int vlev);				  //检查临时变量是否存在
bool procedure_existed(string pname, types vtype, int plev);										  //检查过程是否存在
string find_keyword_prefix(string token);															  //查找关键字前缀

void print_word(string token, int type); //打印单词
void input2file();						 //输入文件
void error(string error);				 //错误处理函数

//赋值过程
/*
文法产生式
<程序>→<分程序>
<分程序>→begin <说明语句表>；<执行语句表> end
// <说明语句表>→<说明语句>│<说明语句表> ；<说明语句>
<说明语句表>→<说明语句><说明语句表_>
<说明语句表_>→；<说明语句><说明语句表_>|epsilon
<说明语句>→<变量说明>│<函数说明>
<变量说明>→integer <变量>
<变量>→<标识符>
// <标识符>→<字母>│<标识符><字母>│ <标识符><数字>
<标识符>→<字母><标识符_>
<标识符_>→<字母><标识符_>│ <数字><标识符_>|epsilon
<字母>→a│b│c│d│e│f│g│h│i│j│k│l│m│n│o│p│q│r│s│t│u│v│w│x│y│z|A│B│C│D│E│F│G│H│I│J│K│L│M│N│O│P│Q│R│S│T│U│V│W│X│Y│Z
<数字>→0│1│2│3│4│5│6│7│8│9
<函数说明>→integer function <标识符>（<参数>）；<函数体>
<参数>→<变量>
<函数体>→begin <说明语句表>；<执行语句表> end
// <执行语句表>→<执行语句>│<执行语句表>；<执行语句>
<执行语句表>→<执行语句><执行语句表_>
<执行语句表_>→；<执行语句><执行语句表_>|epsilon
// <执行语句>→<读语句>│<写语句>│<赋值语句>│<条件语句>
<执行语句>→<读语句>│<写语句>│<赋值语句>│<条件语句>|<返回语句>
<读语句>→read(<变量>)
<写语句>→write(<变量>)
<赋值语句>→<变量>:=<算术表达式>
<返回语句>→<标识符> := <算术表达式>//此处添加返回语句产生式
// <算术表达式>→<算术表达式>-<项>│<项>
<算术表达式>→<项><算术表达式_>
<算术表达式_>→-<项><算术表达式_>│epsilon
// <项>→<项>*<因子>│<因子>
<项>→<因子><项_>
<项_>→*<因子><项_>│epsilon
<因子>→<变量>│<常数>│<函数调用>
<常数>→<无符号整数>
// <无符号整数>→<数字>│<无符号整数><数字>
<无符号整数>→<数字><无符号整数_>
<无符号整数_>→<数字><无符号整数_>|epsilon
<条件语句>→if<条件表达式>then<执行语句>else <执行语句>
<条件表达式>→<算术表达式><关系运算符><算术表达式>
<关系运算符> →<│<=│>│>=│=│<>
<函数调用>→<标识符>(<参数>)//缺少该文法
*/

void program();						 //程序
void subprogram();					 //分程序
void declaration_statement_table();	 //说明语句表
void declaration_statement_table_(); //说明语句表_
void exec_statement_table();		 //执行语句表
void exec_statement_table_();		 //执行语句表_
void declaration_statement();		 //说明语句
void variable_declaration();		 //变量说明
void function_declaration();		 //函数说明
void variable();					 //变量
void identifier();					 //标识符
void identifier_();					 //标识符_
void alphabet();					 //字母
void digit();						 //数字
void parameter();					 //参数
void function_body();				 //函数体
void exec_statement();				 //执行语句
void read_statement();				 //读语句
void write_statement();				 //写语句
void assignment_statement();		 //赋值语句
void return_statement();			 //返回语句
void condition_statement();			 //条件语句
void arithmetic_expression();		 //算术表达式
void arithmetic_expression_();		 //算术表达式_
void term();						 //项
void term_();						 //项_
void factor();						 //因子
void constant();					 //常数
void function_call();				 //函数调用
void unsigned_integer();			 //无符号整数
void unsigned_integer_();			 //无符号整数_
void condition_expression();		 //条件表达式
void relation_operator();			 //关系运算符

FILE *fp1 = fopen("../../out/test.dys", "w");
FILE *fp2 = fopen("../../out/test.err", "w");
FILE *fp3 = fopen("../../out/test.var", "w");
FILE *fp4 = fopen("../../out/test.pro", "w");

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

	freopen("../../out/test.dyd", "r", stdin);
	if (fp1 == NULL)
	{
		printf("test.dys open file error\n");
		return 0;
	}
	if (fp2 == NULL)
	{
		printf("test.err open file error\n");
		return 0;
	}
	if (fp3 == NULL)
	{
		printf("test.var open file error\n");
		return 0;
	}
	if (fp4 == NULL)
	{
		printf("test.pro open file error\n");
		return 0;
	}
	get_next_token();
	program();
	input2file();
	fclose(fp1);
	fclose(fp2);
	fclose(fp3);
	fclose(fp4);

	return 0;
}

void get_next_token()
{
	old_token = token;
	old_type = type;
	cin >> token >> type;
	print_word(token, type);
	if (token == "EOLN")
	{
		line_num++;
		cin >> token >> type;
		print_word(token, type);
	}
	retract_flag = false;
}

void get_next_line()
{
	int current_line = line_num;
	while (current_line == line_num)
	{
		get_next_token();
	}
}

void match_token(string t)
{
	if (t == token)
	{
		get_next_token();
	}
	else
	{
		error("missing `" + t + "`");
	}
}

void assign_variable(string vname, string vproc, bool vkind, types vtype, int vlev, int vadr)
{
	v_tmp.vname = vname;
	v_tmp.vproc = vproc;
	v_tmp.vkind = vkind;
	v_tmp.vtype = vtype;
	v_tmp.vlev = vlev;
	v_tmp.vadr = vadr;
}

void assign_procedure(string pname, types vtype, int plev, int fadr, int ladr)
{
	p_tmp.pname = pname;
	p_tmp.vtype = vtype;
	p_tmp.plev = plev;
	p_tmp.fadr = fadr;
	p_tmp.ladr = ladr;
}

void add_variable_table(string vname, string vproc, bool vkind, types vtype, int vlev)
{
	assign_variable(vname, vproc, vkind, vtype, vlev, variable_table.size());
	variable_table.push_back(v_tmp);
}

void add_tmp_variable_table(string vname, string vproc, bool vkind, types vtype, int vlev)
{

	assign_variable(vname, vproc, vkind, vtype, vlev, tmp_variable_table.size());
	tmp_variable_table.push_back(v_tmp);
}

void add_procedure_table(string pname, types vtype, int plev, int fadr, int ladr)
{
	assign_procedure(pname, vtype, plev, fadr, ladr);
	procedure_table.push_back(p_tmp);
}

void add_tmp_procedure_table(string pname, types vtype, int plev, int fadr, int ladr)
{
	assign_procedure(pname, vtype, plev, fadr, ladr);
	tmp_procedure_table.push_back(p_tmp);
}

bool variable_existed(string vname, string vproc, bool vkind, types vtype, int vlev)
{
	for (int i = 0; i < variable_table.size(); i++)
	{
		if (variable_table[i].vname == vname && variable_table[i].vproc == vproc && variable_table[i].vkind == vkind && variable_table[i].vtype == vtype && variable_table[i].vlev == vlev)
		{
			return true;
		}
	}
	for (int i = 0; i < tmp_variable_table.size(); i++)
	{
		if (tmp_variable_table[i].vname == vname && tmp_variable_table[i].vproc == vproc && tmp_variable_table[i].vkind == vkind && tmp_variable_table[i].vtype == vtype && tmp_variable_table[i].vlev == vlev)
		{
			return true;
		}
	}
	return false;
}

bool find_tmp_variable(string vname, string vproc, bool vkind, types vtype, int vlev)
{
	for (int i = 0; i < tmp_variable_table.size(); i++)
	{
		if (tmp_variable_table[i].vname == vname && tmp_variable_table[i].vproc == vproc && tmp_variable_table[i].vkind == vkind && tmp_variable_table[i].vtype == vtype && tmp_variable_table[i].vlev == vlev)
		{
			return true;
		}
	}
	return false;
}

bool procedure_existed(string pname, types vtype)
{
	for (int i = 0; i < procedure_table.size(); i++)
	{
		if (procedure_table[i].pname == pname && procedure_table[i].vtype == vtype)
		{
			return true;
		}
	}
	for (int i = 0; i < tmp_procedure_table.size(); i++)
	{
		if (tmp_procedure_table[i].pname == pname && tmp_procedure_table[i].vtype == vtype)
		{
			return true;
		}
	}
	for (int i = 0; i < procedure_stack.size(); i++)
	{
		if (procedure_stack[i].pname == pname && procedure_stack[i].vtype == vtype)
		{
			return true;
		}
	}
	return false;
}

string find_keyword_prefix(string token)
{
	for (int i = 0; i < 9; i++)
	{
		if (keyword[i] == token.substr(0, keyword[i].length()))
		{
			return keyword[i];
		}
	}
	return token;
}

void retract()
{
	//此处需要将token和type使用空格分开
	sprintf(ch, "%16s %2d", token.c_str(), type);
	for (int i = strlen(ch) - 1; i >= 0; i--)
	{
		ungetc(ch[i], stdin);
	}
	token = old_token;
	type = old_type;
	retract_flag = true;
}

bool is_letter(char ch)
{
	return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z');
}

bool is_digit(char ch)
{
	return ch >= '0' && ch <= '9';
}

// <程序>→<分程序>
void program()
{
	subprogram();
}

// <分程序>→begin <说明语句表>；<执行语句表> end
void subprogram()
{
	if (token == "begin")
	{
		match_token("begin");
	}
	else
	{
		error("missing `begin`");
	}
	declaration_statement_table();
	if (token == ";")
	{
		match_token(";");
	}
	else
	{
		error("missing `;`");
	}
	exec_statement_table();
	if (token == "end")
	{
		match_token("end");
	}
	else
	{
		error("missing `end`");
	}
}

// <说明语句表>→<说明语句><说明语句表_>
void declaration_statement_table()
{
	declaration_statement();
	declaration_statement_table_();
}

// <说明语句表_>→；<说明语句><说明语句表_>|epsilon
void declaration_statement_table_()
{
	if (token == ";")
	{
		match_token(";");
		//如果下一条语句仍是说明语句，则继续执行
		if (token == "integer")
		{
			declaration_statement();
			declaration_statement_table_();
		}
		else
		{
			retract();
		}
	}
}

// <说明语句>→<变量说明>│<函数说明>
void declaration_statement()
{
	if (token == "integer")
	{
		match_token("integer");
	}
	else
	{
		error("missing `integer`");
		get_next_token();
	}
	if (token == "function")
	{
		function_declaration();
	}
	else
	{
		variable_declaration();
	}
}

// <变量说明>→integer <变量>
void variable_declaration()
{
	if (variable_existed(token, current_proc_name, false, integer, level))
	{
		error("variable `" + token + "` has been declared");
	}
	else
	{
		add_variable_table(token, current_proc_name, false, integer, level);
	}

	variable();
}

// <变量>→<标识符>
void variable()
{
	identifier();
}

// <标识符>→<字母><标识符_>
void identifier()
{
	alphabet();
	identifier_();
}

// <标识符_>→<字母><标识符_>│ <数字><标识符_>|epsilon
void identifier_()
{
	if (p_token >= token.length())
	{
		p_token = 0;
		get_next_token();
	}
	else if (is_letter(token[p_token]))
	{
		alphabet();
		identifier_();
	}
	else if (is_digit(token[p_token]))
	{
		digit();
		identifier_();
	}
}

// <字母>→a│b│c│d│e│f│g│h│i│j│k│l│m│n│o│p│q│r│s│t│u│v│w│x│y│z|A│B│C│D│E│F│G│H│I│J│K│L│M│N│O│P│Q│R│S│T│U│V│W│X│Y│Z
void alphabet()
{
	if (is_letter(token[p_token]))
	{
		p_token++;
	}
}

// <数字>→0│1│2│3│4│5│6│7│8│9
void digit()
{
	if (is_digit(token[p_token]))
	{
		p_token++;
	}
}

// <函数说明>→integer function <标识符>（<参数>）；<函数体>
void function_declaration()
{
	if (token == "function")
	{
		match_token("function");
		level++;
		current_proc_name = token;
		//最后一个变量地址暂设为第一个变量地址
		assign_procedure(token, integer, level, variable_table.size(), variable_table.size());
		procedure_stack.push_back(p_tmp);
		identifier();
		if (token == "(")
		{
			match_token("(");
		}
		else
		{
			error("missing `(`");
		}
		add_variable_table(token, current_proc_name, true, integer, level);
		parameter();
		if (token == ")")
		{
			match_token(")");
		}
		else
		{
			error("missing `)`");
		}
		if (token == ";")
		{
			match_token(";");
		}
		else
		{
			error("missing `;`");
		}
		function_body();
		procedure_stack[procedure_stack.size() - 1].ladr = variable_table.size() - 1;
		procedure_table.push_back(procedure_stack[procedure_stack.size() - 1]);
		procedure_stack.pop_back();
		if (procedure_stack.empty())
		{
			current_proc_name = "main";
		}
		else
		{
			current_proc_name = procedure_stack[procedure_stack.size() - 1].pname;
		}
		level--;
	}
	else
	{
		error("missing `function`");
	}
}

// <参数>→<变量>
void parameter()
{
	variable();
}

// <函数体>→begin <说明语句表>；<执行语句表> end
void function_body()
{
	if (token == "begin")
	{
		match_token("begin");
	}
	else
	{
		error("missing `begin`");
	}
	declaration_statement_table();
	if (token == ";")
	{
		match_token(";");
		exec_statement_table();
		if (token == "end")
		{
			match_token("end");
		}
		else
		{
			error("missing `end`");
		}
	}
	else
	{
		error("missing `;`");
	}
}

// <执行语句表>→<执行语句><执行语句表_>
void exec_statement_table()
{
	exec_statement();
	exec_statement_table_();
}

// <执行语句表_>→；<执行语句><执行语句表_>|epsilon
void exec_statement_table_()
{
	if (token == ";")
	{
		match_token(";");
		exec_statement();
		exec_statement_table_();
	}
}

// <执行语句>→<读语句>│<写语句>│<赋值语句>│<条件语句>|<返回语句>
void exec_statement()
{
	if (find_keyword_prefix(token) == "read")
	{
		if (token == "read")
		{
			match_token("read");
		}
		else
		{
			error("`" + token + "` was not declared, did you mean `read`?");
			token = token.substr(4, token.size() - 4);
			type = symbol_map["symbol"];
		}
		read_statement();
	}
	else if (find_keyword_prefix(token) == "write")
	{
		if (token == "write")
		{
			match_token("write");
		}
		else
		{
			error("`" + token + "` was not declared, did you mean `write`?");
			token = token.substr(5, token.size() - 5);
			type = symbol_map["symbol"];
		}
		write_statement();
	}
	else if (token == "if")
	{
		condition_statement();
	}
	else if (type == symbol_map["symbol"] && procedure_existed(token, integer))
	{
		return_statement();
	}
	else
	{
		get_next_token();
		if (token == ":=")
		{
			retract();
			assignment_statement();
		}
		else
		{
			retract();
			//重大出错，直接退出
			error("syntax error");
			input2file();
			fclose(fp1);
			fclose(fp2);
			fclose(fp3);
			fclose(fp4);
			exit(0);
		}
	}
}

// <读语句>→read(<变量>)
void read_statement()
{
	if (token == "(")
	{
		match_token("(");
	}
	else
	{
		error("missing `(`");
	}
	if (!variable_existed(token, current_proc_name, false, integer, level))
	{
		error("variable `" + token + "` was not declared");
		add_tmp_variable_table(token, current_proc_name, false, integer, level);
	}
	variable();
	if (token == ")")
	{
		match_token(")");
	}
	else
	{
		error("missing `)`");
	}
}

// <写语句>→write(<变量>)
void write_statement()
{
	if (token == "(")
	{
		match_token("(");
	}
	else
	{
		error("missing `(`");
	}
	if (!variable_existed(token, current_proc_name, false, integer, level))
	{
		error("variable `" + token + "` was not declared");
		add_tmp_variable_table(token, current_proc_name, false, integer, level);
	}
	variable();
	if (token == ")")
	{
		match_token(")");
	}
	else
	{
		error("missing `)`");
	}
}

// <赋值语句>→<变量>:=<算术表达式>
void assignment_statement()
{
	if (!variable_existed(token, current_proc_name, false, integer, level))
	{
		error("variable `" + token + "` was not declared");
		add_tmp_variable_table(token, current_proc_name, false, integer, level);
	}
	variable();
	if (token == ":=")
	{
		match_token(":=");
		arithmetic_expression();
	}
	else
	{
		error("missing `:=`");
	}
}

// <返回语句>→<标识符> := <算术表达式>
void return_statement()
{
	if (!procedure_existed(token, integer))
	{
		error("procedure `" + token + "` was not declared");
		//以0作为默认值
		add_tmp_procedure_table(token, integer, 0, 0, 0);
	}
	variable();
	if (token == ":=")
	{
		match_token(":=");
		arithmetic_expression();
	}
	else
	{
		error("missing `:=`");
	}
}

// <算术表达式>→<项><算术表达式_>
void arithmetic_expression()
{
	term();
	arithmetic_expression_();
}
// <算术表达式_>→-<项><算术表达式_>│epsilon
void arithmetic_expression_()
{
	if (token == "-")
	{
		match_token("-");
		term();
		arithmetic_expression_();
	}
}

// <项>→<因子><项_>
void term()
{
	factor();
	term_();
}

// <项_>→*<因子><项_>│epsilon
void term_()
{
	if (token == "*")
	{
		match_token("*");
		factor();
		term_();
	}
}

// <因子>→<变量>│<常数>│<函数调用>
void factor()
{
	if (type == symbol_map["symbol"] && !procedure_existed(token, integer))
	{
		if (!variable_existed(token, current_proc_name, false, integer, level))
		{
			error("variable `" + token + "` was not declared");
			add_tmp_variable_table(token, current_proc_name, false, integer, level);
		}
		variable();
	}
	else if (type == symbol_map["constant"])
	{
		constant();
	}
	else
	{
		function_call();
	}
}

// <常数>→<无符号整数>
void constant()
{
	unsigned_integer();
}

// <无符号整数>→<数字><无符号整数_>
void unsigned_integer()
{
	digit();
	unsigned_integer_();
}

// <无符号整数_>→<数字><无符号整数_>|epsilon
void unsigned_integer_()
{
	if (p_token >= token.length())
	{
		p_token = 0;
		get_next_token();
	}
	else if (isdigit(token[p_token]))
	{
		digit();
		unsigned_integer_();
	}
}

// <条件语句>→if<条件表达式>then<执行语句>else <执行语句>
void condition_statement()
{
	if (token == "if")
	{
		match_token("if");
		condition_expression();
		if (token == "then")
		{
			match_token("then");
			exec_statement();
			if (token == "else")
			{
				match_token("else");
				exec_statement();
			}
			else
			{
				error("missing `else`");
			}
		}
		else
		{
			error("missing `then`");
		}
	}
}

// <条件表达式>→<算术表达式><关系运算符><算术表达式>
void condition_expression()
{
	arithmetic_expression();
	relation_operator();
	arithmetic_expression();
}

// <关系运算符> →<│<=│>│>=│=│<>
void relation_operator()
{
	if (token == "<=" || token == ">" || token == "=" || token == "<>" || token == ">=")
	{
		match_token(token);
	}
	else
	{
		error("missing relation operator");
	}
}

// <函数调用>→<标识符>(<算术表达式>)
void function_call()
{
	if (!procedure_existed(token, integer))
	{
		error("procedure `" + token + "` was not declared");
		//以0作为默认值
		add_tmp_procedure_table(token, integer, 0, 0, 0);
	}
	identifier();
	if (token == "(")
	{
		match_token("(");
		arithmetic_expression();
		if (token == ")")
		{
			match_token(")");
		}
		else
		{
			error("missing `)`");
		}
	}
	else
	{
		error("missing `(`");
	}
}

void print_word(string token, int type)
{
	if (!retract_flag)
	{
		fprintf(fp1, "%16s %2d\n", token.c_str(), type);
	}
}

void error(string error)
{
	printf("***LINE: %d near '%s' %s\n", line_num, token.c_str(), error.c_str());
	fprintf(fp2, "***LINE: %d near '%s' %s\n", line_num, token.c_str(), error.c_str());
}

void input2file()
{
	for (int i = 0; i < variable_table.size(); i++)
	{
		string vtype = (variable_table[i].vtype == integer) ? "integer" : "";
		// printf("%16s %16s %d %s %d %d\n", variable_table[i].vname.c_str(), variable_table[i].vproc.c_str(), variable_table[i].vkind, vtype.c_str(), variable_table[i].vlev, variable_table[i].vadr);
		fprintf(fp3, "%16s %16s %d %s %d %d\n", variable_table[i].vname.c_str(), variable_table[i].vproc.c_str(), variable_table[i].vkind, vtype.c_str(), variable_table[i].vlev, variable_table[i].vadr);
	}
	for (int i = 0; i < procedure_table.size(); i++)
	{
		string ptype = (procedure_table[i].vtype == integer) ? "integer" : "";
		// printf("%16s %s %d %d %d\n", procedure_table[i].pname.c_str(), ptype.c_str(), procedure_table[i].plev, procedure_table[i].fadr, procedure_table[i].ladr);
		fprintf(fp4, "%16s %s %d %d %d\n", procedure_table[i].pname.c_str(), ptype.c_str(), procedure_table[i].plev, procedure_table[i].fadr, procedure_table[i].ladr);
	}
}