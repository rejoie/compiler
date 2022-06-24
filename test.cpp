/*
 * @Date: 2022-05-14 21:48:15
 * @LastEditors: tang hao
 * @LastEditTime: 2022-05-17 22:15:40
 * @FilePath: /code/test.cpp
 */
#include <bits/stdc++.h>
using namespace std;
int main()
{
    // map<string, int> symbol_map;
    // symbol_map["begin"] = 1;
    // symbol_map["end"] = 2;
    // symbol_map["integer"] = 3;
    // symbol_map["if"] = 4;
    // symbol_map["then"] = 5;
    // symbol_map["else"] = 6;
    // symbol_map["function"] = 7;
    // symbol_map["read"] = 8;
    // symbol_map["write"] = 9;
    // symbol_map["symbol"] = 10;
    // symbol_map["const"] = 11;
    // symbol_map["="] = 12;
    // symbol_map["<>"] = 13;
    // symbol_map["<="] = 14;
    // symbol_map["<"] = 15;
    // symbol_map[">="] = 16;
    // symbol_map[">"] = 17;
    // symbol_map["-"] = 18;
    // symbol_map["*"] = 19;
    // symbol_map[":="] = 20;
    // symbol_map["("] = 21;
    // symbol_map[")"] = 22;
    // symbol_map[";"] = 23;
    // symbol_map["EOLN"] = 24;
    // symbol_map["EOF"] = 25;
    // map<string, int>::iterator it;
    // for (it = symbol_map.begin(); it != symbol_map.end(); it++)
    // {
    //     cout << it->first << " " << it->second << endl;
    // }
    freopen("./out/test.dyd", "r", stdin);
    string token;
    char ch[20];
    int type;
    // while (cin >> token >> type)
    // {
    //     cout << token << " " << type << endl;
    // }
    cin >> token >> type;
    cout << token << " " << type << endl;
    sprintf(ch, "%16s%2d", token.c_str(), type);
    for (int i = strlen(ch) - 1; i >= 0; i--)
    {
        ungetc(ch[i], stdin);
    }
    cin >> token >> type;
    cout << token << " " << type << endl;
    bool flag = true;
    printf("%d\n", flag);
    typedef enum
    {
        integer
    } types; //支持类型的集合
    types a=integer;
    printf("%s\n", a);
    return 0;
}
