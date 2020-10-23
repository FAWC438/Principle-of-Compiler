#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

int line = 0, column = 0, cnt_word = 0, cnt_char = 0;

ifstream in_file_stream;
ofstream out_file_stream;

string in_file_str, out_file_str, buffer, token;
string::iterator ptr_forward = buffer.end(); // 向前指针
string words[] = {"include", "define", "auto", "double", "int", "struct", "break", "else", "long", "switch", "case", "enum", "register", "typedef", "char", "extern", "return", "union", "const", "float", "short", "unsigned", "continue", "for", "signed", "void", "default", "goto", "sizeof", "volatile", "do", "if", "static", "while"};
vector<string> table;
map<string, int> counter_map;

/**
 * @brief 向符号表中插入符号。这个符号可能已经存在，也可能是新符号。
 * 若为新符号，插入符号表末尾。
 * 
 * @return int 返回插入的符号在符号表的位置，若是新符号，则一定在符号表的末尾
 */
int table_insert()
{
    vector<string>::iterator it = find(table.begin(), table.end(), token);

    if (it == table.end())
    {
        table.push_back(token);
        it = find(table.begin(), table.end(), token);
    }

    return distance(table.begin(), it) + 1;
}

/**
 * @brief 读取并测试实数，其中包含浮点数或带有E/e的指数。
 * 注意，本函数涉及状态转换。
 * 状态1为初始状态，读取实数；
 * 状态2为读取小数点后数字的状态；
 * 状态3为读取指数符号后数字的状态；
 * 状态4为读取指数符号后的缓冲态，以防止指数符号后出现+/-符号；
 * 状态5为读取指数符号后数字的状态。
 * 
 * @param state_param 状态参数，输入1则开始判断实数
 * @return true 发生了错误
 * @return false 未发生错误
 */
bool test_digits(int state_param)
{
    int state = state_param;
    bool not_end_boolen = true, is_error = false;

    while ((ptr_forward != buffer.end()) && not_end_boolen)
    {
        switch (state)
        {
        case 1:
            // 数字部分
            if (*ptr_forward == '.')
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 2;
            }
            else if (*ptr_forward == 'E' || *ptr_forward == 'e')
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 3;
            }
            else if (isdigit(*ptr_forward))
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 1;
            }
            else
                not_end_boolen = false; // 识别结束

            break;

        case 2:
            // 读取到小数点
            if (token.find('.') != string::npos)
            {
                out_file_stream << "< Error(" << line << "," << column << "): Invalid number pattern (double dots) >" << endl;
                is_error = true;
                not_end_boolen = false;
            }
            else if (*ptr_forward == 'E' || *ptr_forward == 'e')
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 3;
            }
            else if (isdigit(*ptr_forward))
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 1;
            }
            else
                not_end_boolen = false;

            break;

        case 3:
            // 读取到指数符号
            if (*ptr_forward == '+' || *ptr_forward == '-')
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 4;
            }
            else if (isdigit(*ptr_forward))
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 5;
            }
            else
            {
                out_file_stream << "< Error(" << line << "," << column << "): Exponent has no digits >" << endl;
                is_error = true;
                not_end_boolen = false;
            }

            break;

        case 4:
            // +/-号后必须有一个数字
            if (isdigit(*ptr_forward))
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 5;
            }
            else
            {
                out_file_stream << "< Error(" << line << "," << column << "): Exponent has no digits >" << endl;
                is_error = true;
                not_end_boolen = false;
            }

            break;

        case 5:
            // 仅数字部分
            if (isdigit(*ptr_forward))
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 5;
            }
            else
                not_end_boolen = false;

            break;

        default:
            // 实际上代码不可能运行到此处
            out_file_stream << "< Error(" << line << "," << column << "): Function test_digits() Error! >" << endl;
            not_end_boolen = false;
            is_error = true;
            break;
        }
    }

    if ((*ptr_forward >= 'a' && *ptr_forward <= 'z') || (*ptr_forward >= 'A' && *ptr_forward <= 'Z') || *ptr_forward == '_')
        return true;

    return is_error;
}

/**
 * @brief 测试注释内容
 * 
 * @return true 注释通过词法分析
 * @return false 读到错误或EOF
 */
bool test_comments()
{
    while (true)
    {
        while ((ptr_forward != buffer.end()) && (*ptr_forward != '*'))
        {
            token.append(1, *ptr_forward++);
            column++;
        }

        if (ptr_forward == buffer.end())
        {
            if (getline(in_file_stream, buffer, '\n'))
            {
                // 一行注释结束，继续下一行
                line++;
                ptr_forward = buffer.begin();
                column = 1;
                continue;
            }
            else
                return false;
        }
        else
        {
            // 读到注释结束符的星号
            token.append(1, *ptr_forward++);
            column++;

            if (ptr_forward == buffer.end()) // 若 '*/' 被换行分开，则继续读取下一行
                continue;
            else if (*ptr_forward == '/')
            {
                // 注释结束
                token.append(1, *ptr_forward++);
                column++;
                return true;
            }
            else // 星号后是其他字符，仍继续读取注释
                continue;
        }
    }
}

/**
 * @brief 封装输出样式
 * 
 * @param type_str 输出的记号类型
 */
void output_line(string type_str)
{
    string type_str_finder = type_str;
    if (type_str_finder.substr(0, 2) == "ID")
        type_str_finder = "ID";

    map<string, int>::iterator iter = counter_map.find(type_str_finder);
    if (iter != counter_map.end())
        counter_map[type_str_finder]++;
    else
        counter_map[type_str_finder] = 1;

    if (type_str == "comments" || type_str == "string")
    {
        out_file_stream << "<"
                        << right << setw(5) << line << ":"
                        << left << setw(10) << column
                        << left << setw(10) << type_str
                        << left << setw(13) << "-"
                        << ">" << endl;
    }
    else
    {
        out_file_stream << "<"
                        << right << setw(5) << line << ":"
                        << left << setw(10) << column
                        << left << setw(10) << type_str
                        << left << setw(13) << token << ">" << endl;
    }
}

/**
 * @brief 在输出开头展示说明文本
 * 
 */
void show_head_word()
{
    out_file_stream << right << setw(25) << "Specification" << endl
                    << endl;
    out_file_stream << "[ID-<number>]: 用户定义或额外导入的库函数中的记号" << endl;
    out_file_stream << "[keyword]: C语言保留字" << endl;
    out_file_stream << "[num]: 全体实数，支持指数表示" << endl;
    out_file_stream << "[comments]: 注释" << endl;
    out_file_stream << "[punct]: 标点符号" << endl;
    out_file_stream << "[char]: 字符" << endl;
    out_file_stream << "[string]: 字符串" << endl;
    out_file_stream << "[arith-op]: 算数运算符" << endl;
    out_file_stream << "[asgn-op]: 复合运算符" << endl;
    out_file_stream << "[ptr-op]: 指针运算符" << endl;
    out_file_stream << "[bit-op]: 位运算符" << endl;
    out_file_stream << "[logic-op]: 逻辑运算符" << endl;
    out_file_stream << "[relop-op]: 关系运算符" << endl
                    << endl;
    out_file_stream << "请注意：以下的Column对于多个字符的记号来说，指向的是其最后一个字符所在的列数" << endl
                    << endl;
    out_file_stream << "----------------Result-------------------" << endl;
    out_file_stream << right << setw(6) << "Line"
                    << ":"
                    << left << setw(10) << "Column"
                    << left << setw(10) << "Type"
                    << left << setw(13) << "Token" << endl;
}

/**
 * @brief 展示结果文本并关闭IO流
 * 
 */
void show_result()
{
    int i = 1;

    out_file_stream << "---------------ID table------------------" << endl;

    for (vector<string>::iterator it = table.begin(); it != table.end(); it++)
        out_file_stream << left << setw(5) << i++ << "\t" << *it << endl;

    i = 1;

    out_file_stream << "---------------Analysis------------------" << endl;

    for (map<string, int>::iterator it = counter_map.begin(); it != counter_map.end(); it++)
        out_file_stream << left << setw(5) << i++ << "\t" << left << setw(10) << it->first << "\t" << it->second << endl;

    cout << "分析完成，请到目标文件" << out_file_str << "查看输出结果！" << endl;
    out_file_stream << "------------------Total------------------" << endl;
    out_file_stream << "Total lines: " << line << endl;
    out_file_stream << "Total words: " << cnt_word << endl;
    out_file_stream << "Total characters: " << cnt_char << endl;

    in_file_stream.close();
    out_file_stream.close();
}

int main()
{
    set<string> keywords(words, words + 34);
    char C;
    cout << "请输入源文件名称（回车则默认使用in.c）：" << endl;
    getline(cin, in_file_str);

    if (in_file_str == "")
        in_file_str = "in.c";

    in_file_stream.open(in_file_str.c_str());

    if (!in_file_stream)
    {
        cout << "无法打开源文件！" << endl;
        return -1;
    }

    cout << "请输入目标文件名称（回车则默认使用out.txt）：" << endl;
    getline(cin, out_file_str);

    if (out_file_str == "")
        out_file_str = "out.txt";

    out_file_stream.open(out_file_str.c_str());

    if (!out_file_stream)
    {
        cout << "无法创建目标文件！" << endl;
        return -1;
    }

    show_head_word();

    while (true)
    {
        // 前进指针读到缓存末尾
        if (ptr_forward == buffer.end())
        {
            // 读到行末
            if (getline(in_file_stream, buffer, '\n'))
            {
                line++;             // 读取新行
                cnt_char += column; // 每次换行前加上当前行的字符数
                ptr_forward = buffer.begin();
                column = 1;
            }
            else // 读到EOF则输出结果
            {
                show_result();
                return 0;
            }
        }

        while ((ptr_forward != buffer.end()) && isspace(*ptr_forward))
        {
            // 跳过空字符
            ptr_forward++;
            column++;
        }

        if (ptr_forward != buffer.end())
        {
            // 未读到缓冲区末尾
            token = "";
            C = *ptr_forward;

            if ((C >= 'a' && C <= 'z') || (C >= 'A' && C <= 'Z') || C == '_')
            {
                token.append(1, C);
                ptr_forward++;
                column++;

                while ((ptr_forward != buffer.end()) && (isalnum(*ptr_forward) || *ptr_forward == '_'))
                {
                    // 合法标识符可包含下划线
                    token.append(1, *ptr_forward++);
                    column++;
                }

                if (keywords.count(token) == 0)
                    output_line("ID-" + to_string(table_insert()));
                else
                    output_line("keyword");

                cnt_word++; // 单词数加一
            }
            else if (C >= '0' && C <= '9')
            {
                token.append(1, C);
                ptr_forward++;
                column++;
                if (!test_digits(1)) // 读取无符号实数剩余部分
                {
                    output_line("num");
                    cnt_word++;
                }
                else
                    out_file_stream << "< Error(" << line << "," << column << "): ID cannot start with digits >" << endl;
            }
            else
            {
                switch (C)
                {
                case '+':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("arith-op");
                    else
                    {
                        if (*ptr_forward == '+')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("arith-op");
                        }
                        else if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("asgn-op");
                        }
                        else
                            output_line("arith-op");
                    }

                    cnt_word++;
                    break;

                case '-':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("arith-op");
                    else
                    {
                        if (*ptr_forward == '-')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("arith-op");
                        }
                        else if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("asgn-op");
                        }
                        else if (*ptr_forward == '>')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("ptr-op");
                        }
                        else
                            output_line("arith-op");
                    }

                    cnt_word++;
                    break;

                case '*':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("arith-op");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("asgn-op");
                        }
                        else
                            output_line("arith-op");
                    }

                    cnt_word++;
                    break;

                case '/':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                    {
                        output_line("arith-op");
                        cnt_word++;
                    }
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            // 除法复合赋值
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("asgn-op");
                            cnt_word++;
                        }
                        else if (*ptr_forward == '/')
                        {
                            // 单行注释，读到行末
                            token.append(1, *ptr_forward++);
                            column++;

                            while (ptr_forward != buffer.end())
                            {
                                token.append(1, *ptr_forward++);
                                column++;
                            }
                            output_line("comments");
                        }
                        else if (*ptr_forward == '*')
                        {
                            // 多行注释可以换行
                            token.append(1, *ptr_forward++);
                            column++;
                            int ret = test_comments();

                            if (ret)
                                output_line("comments");
                            else
                            {
                                show_result();
                                return 0;
                            }
                        }
                        else
                        {
                            // 除号后是其他字符，为单个除号
                            output_line("arith-op");
                            cnt_word++;
                        }
                    }

                    break;

                case '%':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("arith-op");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("asgn-op");
                        }
                        else
                            output_line("arith-op");
                    }

                    cnt_word++;
                    break;

                case '&':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("bit-op");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("asgn-op");
                        }
                        else if (*ptr_forward == '&')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("logic-op");
                        }
                        else
                            output_line("bit-op");
                    }

                    cnt_word++;
                    break;

                case '|':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("bit-op");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("asgn-op");
                        }
                        else if (*ptr_forward == '|')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("logic-op");
                        }
                        else
                            output_line("bit-op");
                    }

                    cnt_word++;
                    break;

                case '^':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("bit-op");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("asgn-op");
                        }
                        else
                            output_line("bit-op");
                    }

                    cnt_word++;
                    break;

                case '~':
                    token.append(1, C);
                    ptr_forward++;
                    column++;
                    output_line("bit-op");
                    cnt_word++;
                    break;

                case '<':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("relop-op");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("relop-op");
                        }
                        else
                            output_line("relop-op");
                    }

                    cnt_word++;
                    break;

                case '=':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("asgn-op");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("relop-op");
                        }
                        else
                            output_line("asgn-op");
                    }

                    ++cnt_word;
                    break;

                case '>':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("relop-op");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("relop-op");
                        }
                        else
                            output_line("relop-op");
                    }

                    cnt_word++;
                    break;

                case '!':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        output_line("punct");
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("relop-op");
                        }
                        else
                            output_line("punct");
                    }

                    cnt_word++;
                    break;

                case '\"':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    while (true)
                    {
                        while ((ptr_forward != buffer.end()) && (*ptr_forward != '\"'))
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                        }

                        if (ptr_forward == buffer.end())
                        {
                            out_file_stream << "< Error(" << line << "," << column << "): Missing terminating \" character >" << endl;
                            break;
                        }
                        else if (*(ptr_forward - 1) == '\\')
                        {
                            // 跳过转义符\"
                            token.append(1, *ptr_forward++);
                            column++;
                            continue;
                        }
                        else
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            output_line("string");
                            break;
                        }
                    }

                    cnt_word++;
                    break;

                case '\'':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    while ((ptr_forward != buffer.end()) && (*ptr_forward != '\''))
                    {
                        token.append(1, *ptr_forward++);
                        column++;
                    }

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< Error(" << line << "," << column << "): Missing terminating \' character >" << endl;
                    else if ((*(ptr_forward - 2) == '\\' && token.size() == 3) || (*(ptr_forward - 2) != '\\' && token.size() == 2))
                    {
                        // char类型只能是一个字符，或是两个字符的转义符
                        token.append(1, *ptr_forward++);
                        column++;
                        output_line("char");
                        cnt_word++;
                    }
                    else
                    {
                        ptr_forward++;
                        column++;
                        out_file_stream << "< Error(" << line << "," << column << "): Invalid char value >" << endl;
                    }

                    break;

                case '.':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if ((ptr_forward != buffer.end()) && isdigit(*ptr_forward))
                    {
                        // 小数点
                        token.append(1, *ptr_forward++);
                        column++;
                        if (!test_digits(2)) // 读取无符号实数剩余部分
                        {
                            output_line("num");
                            cnt_word++;
                        }
                        else
                            out_file_stream << "< Error(" << line << "," << column << "): ID cannot start with digits >" << endl;
                        break;
                    }

                    output_line("punct");
                    cnt_word++;
                    break;

                case '#':
                case '{':
                case '}':
                case '[':
                case ']':
                case '(':
                case ')':
                case '?':
                case ':':
                case ',':
                case ';':
                case '\\':
                    token.append(1, C);
                    ptr_forward++;
                    column++;
                    output_line("punct");
                    cnt_word++;
                    break;

                default:
                    ptr_forward++;
                    column++;
                    out_file_stream << "< Error(" << line << "," << column << "): Invalid character >" << endl;
                    break;
                } // end of switch
            }     // end of else
        }         // end of if
    }             // end of while

    return 0;
}
