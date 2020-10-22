#include <algorithm>
#include <cctype>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <set>
#include <string>
#include <vector>

using namespace std;

int line = 0, column = 0, cnt_word = 0, cnt_char = 0;

string in_file_str, out_file_str, buffer, token;
string::iterator ptr_forward = buffer.end();
ifstream in_file_stream;
ofstream out_file_stream, table_file_stream;

string words[] = {"include", "define", "auto", "double", "int", "struct", "break", "else", "long", "switch", "case", "enum", "register", "typedef", "char", "extern", "return", "union", "const", "float", "short", "unsigned", "continue", "for", "signed", "void", "default", "goto", "sizeof", "volatile", "do", "if", "static", "while"};
vector<string> table; // 符号表

/**
 * @brief 向符号表中插入符号。这个符号可能已经存在，也可能是新符号。若为新符号，插入符号表末尾。
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
 */
void test_digits(int state_param)
{
    int state = state_param;
    bool not_end_boolen = true;

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
                state = 23;
            }
            else if (*ptr_forward == 'E' || *ptr_forward == 'e')
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 4;
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
            if (*ptr_forward == 'E' || *ptr_forward == 'e')
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 4;
            }
            else if (isdigit(*ptr_forward))
            {
                token.append(1, *ptr_forward++);
                column++;
                state = 2;
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
                out_file_stream << "< Error(" << line << "," << column << "): exponent has no digits >" << endl; //error();
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
                out_file_stream << "< Error(" << line << "," << column << "): exponent has no digits >" << endl; //error();
                not_end_boolen = false;
            }

            break;

        case 5:
            // 指数数字部分
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
            out_file_stream << "< Error(" << line << "," << column << "): function test_digits() Error! >" << endl;
            break;
        }
    }
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

int main()
{
    set<string> keywords(words, words + 34);
    char C;
    cout << "源文件（回车默认使用in.c）：" << endl;
    getline(cin, in_file_str);

    if (in_file_str == "")
        in_file_str = "in.c"; // 回车默认

    in_file_stream.open(in_file_str.c_str());

    if (!in_file_stream)
    {
        cout << "无法打开源文件！" << endl;
        return -1;
    }

    cout << "目标文件（回车默认使用out.txt）：" << endl;
    getline(cin, out_file_str);

    if (out_file_str == "")
        out_file_str = "out.txt"; // 回车默认

    out_file_stream.open(out_file_str.c_str());

    if (!out_file_stream)
    {
        cout << "无法创建目标文件！" << endl;
        return -1;
    }

    table_file_stream.open("符号表.txt");

    if (!table_file_stream)
    {
        cout << "无法创建符号表！" << endl;
        return -1;
    }

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
            else // 读到错误或EOF则返回false
            {
                int i = 1;

                in_file_stream.close();
                out_file_stream.close();

                for (vector<string>::iterator it = table.begin(); it != table.end(); it++)
                    table_file_stream << left << i++ << "\t" << *it << endl;

                table_file_stream.close();
                cout << "总行数：" << line << endl;
                cout << "单词数：" << cnt_word << endl;
                cout << "字符数：" << cnt_char << endl;
                return 0;
            }
        }

        while ((ptr_forward != buffer.end()) && isspace(*ptr_forward))
        {
            // 未读到缓冲区结束，则一直读到非空字符
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
                    out_file_stream << "< ID," << table_insert() << " >" << endl;
                else
                    out_file_stream << "< key," << token << " >" << endl;

                cnt_word++; // 单词数加一
            }
            else if (C >= '0' && C <= '9')
            {
                token.append(1, C);
                ptr_forward++;
                column++;
                test_digits(1);                                       // 读取无符号实数剩余部分
                out_file_stream << "< num," << token << " >" << endl; // DTB(token)
                cnt_word++;
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
                        out_file_stream << "< arith-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '+')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< arith-op," << token << " >" << endl;
                        }
                        else if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< asgn-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< arith-op," << token << " >" << endl;
                    }

                    cnt_word++;
                    break;

                case '-':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< arith-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '-')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< arith-op," << token << " >" << endl;
                        }
                        else if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< asgn-op," << token << " >" << endl;
                        }
                        else if (*ptr_forward == '>')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< ptr-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< arith-op," << token << " > " << endl;
                    }

                    cnt_word++;
                    break;

                case '*':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< arith-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< asgn-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< arith-op," << token << " >" << endl;
                    }

                    cnt_word++;
                    break;

                case '/':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                    {
                        out_file_stream << "< arith-op," << token << " >" << endl; // 行末除号
                        cnt_word++;
                    }
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            // 除法复合赋值
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< asgn-op," << token << " >" << endl;
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

                            out_file_stream << "< comments,"
                                            << "- >" << endl;
                        }
                        else if (*ptr_forward == '*')
                        {
                            // 多行注释可以换行
                            token.append(1, *ptr_forward++);
                            column++;
                            int ret = test_comments();

                            if (ret)
                                out_file_stream << "< comments,"
                                                << "- >" << endl;
                            else
                            {
                                int i = 1;
                                in_file_stream.close();
                                out_file_stream.close();

                                for (vector<string>::iterator it = table.begin(); it != table.end(); it++)
                                    table_file_stream << left << i++ << "\t" << *it << endl;

                                table_file_stream.close();
                                cout << "总行数：" << line << endl;
                                cout << "单词数：" << cnt_word << endl;
                                cout << "字符数：" << cnt_char << endl;
                                return 0;
                            }
                        }
                        else
                        {
                            // 除号后是其他字符，为单个除号
                            out_file_stream << "< arith-op," << token << " >" << endl;
                            cnt_word++;
                        }
                    }

                    break;

                case '%':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< arith-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< asgn-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< arith-op," << token << " >" << endl;
                    }

                    cnt_word++;
                    break;

                case '&':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< bit-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< asgn-op," << token << " >" << endl;
                        }
                        else if (*ptr_forward == '&')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< log-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< bit-op," << token << " >" << endl;
                    }

                    cnt_word++;
                    break;

                case '|':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< bit-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< asgn-op," << token << " >" << endl;
                        }
                        else if (*ptr_forward == '|')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< log-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< bit-op," << token << " >" << endl;
                    }

                    cnt_word++;
                    break;

                case '^':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< bit-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< asgn-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< bit-op," << token << " >" << endl;
                    }

                    cnt_word++;
                    break;

                case '~':
                    token.append(1, C);
                    ptr_forward++;
                    column++;
                    out_file_stream << "< bit-op," << token << " >" << endl;
                    cnt_word++;
                    break;

                case '<':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< rel-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< rel-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< rel-op," << token << " >" << endl;
                    }

                    cnt_word++;
                    break;

                case '=':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< asgn-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< rel-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< asgn-op," << token << " >" << endl;
                    }

                    ++cnt_word;
                    break;

                case '>':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< rel-op," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< rel-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< rel-op," << token << " >" << endl;
                    }

                    cnt_word++;
                    break;

                case '!':
                    token.append(1, C);
                    ptr_forward++;
                    column++;

                    if (ptr_forward == buffer.end())
                        out_file_stream << "< punct," << token << " >" << endl;
                    else
                    {
                        if (*ptr_forward == '=')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< rel-op," << token << " >" << endl;
                        }
                        else
                            out_file_stream << "< punct," << token << " >" << endl;
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
                            out_file_stream << "< Error(" << line << "," << column << "): missing terminating \" character >" << endl; //error();
                            break;
                        }
                        else if (*(ptr_forward - 1) == '\\')
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            continue;
                        }
                        else
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                            out_file_stream << "< string,"
                                            << "- >" << endl;
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
                        // TODO: char 仅能检测单个字符
                        token.append(1, *ptr_forward++);
                        column++;
                    }

                    if (ptr_forward == buffer.end())
                    {
                        out_file_stream << "< Error(" << line << "," << column << "): missing terminating \' character >" << endl; //error();
                        break;
                    }
                    else if (*(ptr_forward - 1) == '\\')
                    {
                        token.append(1, *ptr_forward++);
                        column++;

                        if (ptr_forward != buffer.end())
                        {
                            token.append(1, *ptr_forward++);
                            column++;
                        }
                        else
                            out_file_stream << "< Error(" << line << "," << column << "): missing terminating \' character >" << endl; //error();
                        break;
                    }
                    else
                    {
                        token.append(1, *ptr_forward++);
                        column++;
                    }

                    out_file_stream << "< char,"
                                    << "- >" << endl;
                    cnt_word++;
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
                        test_digits(2);
                        out_file_stream << "< num," << token << " >" << endl; // DTB(token)
                        cnt_word++;
                        break;
                    }

                    out_file_stream << "< punct," << token << " >" << endl;
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
                    out_file_stream << "< punct," << token << " >" << endl;
                    cnt_word++;
                    break;

                default:
                    ptr_forward++;
                    column++;
                    out_file_stream << "< Error(" << line << "," << column << "): Invalid character >" << endl; //error();
                    break;
                } // end of switch
            }     // end of else
        }         // end of if
    }             // end of while

    return 0;
}
