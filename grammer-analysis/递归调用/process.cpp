#include <algorithm>
#include <fstream>
#include <iostream>
#include <string>

using namespace std;

string buffer;
ifstream inputFile("test.txt");
int curCharPointer = 0;
int procE();
int procT();
int procF();

int procE()
{
    int res, E_res, T_res;
    T_res = procT();
    if (buffer[curCharPointer] == '+')
    {
        curCharPointer++;
        E_res = procE();
        cout << "\tE -> E + T: ";
        res = E_res + T_res;
    }
    else if (buffer[curCharPointer] == '-')
    {
        curCharPointer++;
        E_res = procE();
        cout << "\tE -> E - T: ";
        res = T_res - E_res;
    }
    else
    {
        cout << "\tE -> T: ";
        res = T_res;
    }
    cout << res << endl;
    return res;
}

int procT()
{
    int res, F_res, T_res;
    F_res = procF();
    if (buffer[curCharPointer] == '*')
    {
        curCharPointer++;
        T_res = procT();
        cout << "\tT -> T * F: ";
        res = F_res * T_res;
    }
    else if (buffer[curCharPointer] == '/')
    {
        curCharPointer++;
        T_res = procT();
        if (T_res == 0)
        {
            cerr << "Input Data error: Division by zero" << endl;
            exit(1);
        }
        cout << "\tT -> T / F: ";
        res = F_res / T_res;
    }
    else
    {
        cout << "\tT -> F: ";
        res = F_res;
    }
    cout << res << endl;
    return res;
}

int procF()
{
    int res;
    if (buffer[curCharPointer] == '(')
    {
        curCharPointer++;
        int temp = procE();
        if (buffer[curCharPointer] == ')')
        {
            cout << "\tF -> (E): ";
            res = temp;
            curCharPointer++;
        }
        else
        {
            cerr << "Syntax error: Paren matching error" << endl;
            exit(1);
        }
    }
    else if (buffer[curCharPointer] <= '9' && buffer[curCharPointer] >= '0')
    {
        int begin = curCharPointer, len = 0;
        while (buffer[curCharPointer] <= '9' && buffer[curCharPointer] >= '0')
        {
            len++;
            curCharPointer++;
        }
        res = stoi(buffer.substr(begin, len));
        cout << "\tF -> num: ";
    }
    else
    {
        cerr << "Syntax error: Number format error" << endl;
        exit(1);
    }
    cout << res << endl;
    return res;
}

int main(int argc, char const *argv[])
{
    inputFile >> buffer;
    // 删除字符串的空格
    buffer.erase(remove_if(buffer.begin(), buffer.end(), [](unsigned char x) { return isspace(x); }), buffer.end());
    cout << "Buffer: " << buffer << endl;
    int result = procE();
    cout << "Answer: " << result << endl;

    return 0;
}
