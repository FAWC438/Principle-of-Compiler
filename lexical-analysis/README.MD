# 说明文档

| 姓名   | 学号       | 班级       |
| ------ | ---------- | ---------- |
| 凌国瀚 | 2018213344 | 2018211314 |

---

## 目录

<!-- TOC -->

> _1._ [题目要求](#题目要求)
>
> _2._ [开发环境与运行环境](#开发环境与运行环境)
>
> _3._ [程序设计说明](#程序设计说明)
>
> - [文件说明](#文件说明)
> - [运行说明](#运行说明)
> - [程序简介](#程序简介)
> - [数据结构](#数据结构)
> - [函数结构](#函数结构)
>
> _4._ [运行结果及分析](#运行结果及分析)
>
> - [输入](#输入)
> - [运行结果](#运行结果)
> - [分析说明](#分析说明)
>
> _5._ [源程序](#源程序)
>
> - [C++](#C++)
> - [LEX](#LEX)

<!-- /TOC -->

---

## 题目要求

### 实验内容及要求

1. 可以识别出用 C 语言编写的源程序中的每个单词符号，并以记号的形式输出每个单词符号。

2. 可以识别并跳过源程序中的注释。

3. 可以统计源程序中的语句行数、各类单词的个数、以及字符总数，并输出统计结果。

4. 检查源程序中存在的词法错误，并报告错误所在的位置。

5. 对源程序中出现的错误进行适当的恢复，使词法分析可以继续进行，对源程序进行一次扫描，即可检查并报告源程序中存在的所有词法错误。

### 实现方法要求

分别用以下两种方法实现

- 方法一：

  - 采用 C/C++ 作为实现语言，手工编写词法分析程序。

- 方法二：

  - 编写 LEX 源程序，利用 LEX 编译程序自动生成词法分析程序。

---

## 开发环境与运行环境

_代码编辑器_：Visual Studio Code 15.0 with Remote Process Explorer

_操作系统_：Windows 10 (version: 2004)

_运行环境_：Windows Subsystem of Linux (Ubuntu-20.04 LTS)

_编译器_：GCC-9 / G++-5 on Linux

_词法分析器_：flex on Linux

---

## 程序设计说明

### 文件说明

    2018211314-凌国瀚-2018213344
    ├── CPP
    |   ├── analysis
    |   ├── analysis.cpp
    |   ├── in.c
    |   └── out.txt
    ├── LEX
    |   ├── a.out
    |   ├── lex_source.l
    |   ├── lex.yy.c
    |   ├── result.txt
    |   └── test1.c
    └── README.pdf

文件树如上所示，其中`CPP`文件夹存放 C++ 版本的词法分析程序，`LEX`文件夹存放 LEX 版本的词法分析程序。

`CPP`文件夹中，`analysis.cpp`为源代码，`analysis`为可执行文件，`in.c`是默认的待分析文本，`out.txt`是程序输出结果。

`LEX`文件夹中，`lex_source.l`为源代码，`lex.yy.c`为 flex 编译 LEX 文件后生成的中间 C 代码，`a.out`是可执行文件，`test1.c`是默认的待分析文本，`result.txt`是程序输出结果。

`README.md`为本说明文档，`README.pdf`是本文档的 pdf 版本，以便于在没有 Markdown 浏览条件时阅读本文档。

欲运行本程序，请参照[运行说明](#运行说明)。

### 运行说明

#### C++

欲编译本程序并生成可执行文件，请在**源程序所在目录**的 Linux bash 中输入如下指令：

    g++ analysis.cpp -o analysis

即可生成可执行文件`analysis`。

随后，直接使用以下指令即可运行可执行文件：

    ./analysis

#### LEX

欲编译本程序并生成可执行文件，请在**源程序所在目录**的 Linux bash 中依次输入如下指令：

    flex lex_source.l
    gcc lex.yy.c -ll
    ./a.out < test1.cpp

即可生成结果`result.txt`。

> **温馨提示**
>
> 1. 最后一行编译指令中，文件重定向可以更改，以选择`test1.cpp`以外的待分析文本。
> 2. 第二行指令中不要忘记链接 flex 库（-ll）。

### 程序简介

C++版本的程序源文件内容**每次一行**地读入缓冲 buffer 中，向前指针则在每次 buffer 更新后从 buffer 的头部开始扫描，根据扫描得到不同的字符调用相应的分析代码。缓冲 buffer 的设置保证了读入字符的正确性，同时，本程序还支持判断单行和多行注释，并跳过其内容；也支持错误检测和错误处理，支持检测的错误如下：

1. _Exponent has no digits_：指数部分没有数字，如`2.3e`、`4e+`等

2. _ID cannot start with digits_：记号不能以数字开头，如`123abc`、`666_bcd`等

3. _Missing terminating \" character_：字符串未匹配到末尾的冒号

4. _Missing terminating \' character_：字符未匹配到末尾的冒号

5. _Invalid char value_：非法的字符，正确的字符格式为`'a'`、`'\n'`等，非法的字符格式为`'a1'`、`'b\t'`等

### 数据结构

> 本部分仅讨论 C++ 版本程序

#### 计数变量

```cpp
int line = 0, column = 0, cnt_word = 0, cnt_char = 0;
```

计数变量用于记录当前指针所分析的字符在源文件中的行数和列数，也记录了源文件中的单词数量和字符数量。**请注意，被分析程序判断为非法/错误的单词将不参与计数**。

#### IO 流

```cpp
ifstream in_file_stream;
ofstream out_file_stream;
```

本程序利用 C++特有的 IO 流处理文件输入和输出。

#### 缓冲区

```cpp
string buffer, token;
```

利用 C++ 的`string`对象作为缓冲区的数据结构。

`buffer`是数据缓冲区，在程序中，`buffer`存储了当前扫描行数的一整行内容。它的主要功能是超前扫描若干个字符，其目的是为了得到某一个单词符号的确切性质。

`token`是已经扫描成功的字符，当一个单词被认为扫描完毕，此时就应该输出`token`，因为它存储了一个单词的全部字符。

#### 向前指针

```cpp
string::iterator ptr_forward = buffer.end();
```

`ptr_forward`是向前指针，指示了当前即将分析的字符。它的数据类型是`string::iterator`，用以适应`buffer`的数据类型`string`。

#### 关键字集合

```cpp
string words[] = {"include", "define", "auto", "double", "int", "struct", "break", "else", "long", "switch", "case", "enum", "register", "typedef", "char", "extern", "return", "union", "const", "float", "short", "unsigned", "continue", "for", "signed", "void", "default", "goto", "sizeof", "volatile", "do", "if", "static", "while"};
set<string> keywords(words, words + 34);
```

利用 C++ 数据结构`set`，定义关键字集合。这样定义的原因在于，当分析器扫描到一个记号时，可以直接使用`set`的内置函数判断其是否为 C 语言关键字。

#### 分析容器

```cpp
vector<string> table;
map<string, int> counter_map;
```

`table`是一个`vector`容器，其记录了每一个记号 ( ID ) 的出现次数，这个容器的设计目的在于，当文本中多次出现同样模式的记号时，将会将它们归结为同一个记号而非多个记号。

`counter_map`是一个`map`容器，用于统计各类单词的个数。

### 函数结构

> 本部分仅讨论 C++ 版本程序

---

## 运行结果及分析

依照[运行说明](#运行说明)所述方式运行程序后，在控制台中输入数字`35`，得到结果如下：

    kevin_ling@Spear-of-Adun:~/Operating-System/ch4_homework$ ./thread
    35
    The average value is 35
    The minimum value is 35
    The maximum value is 35
    kevin_ling@Spear-of-Adun:~/Operating-System/ch4_homework$

下方流程图有助于理解本程序的运行过程

![avatar](https://github.com/FAWC-bupt/Operating-System/blob/main/ch4_homework/flow_chart.jpg)

从流程图可以看出，在多线程操作中，由于采用了 pthread_join()函数，主线程将被阻塞，同一时间只有可能运行一个线程。一旦子线程结束，主线程就结束阻塞状态，输出子线程的结果并调用另一个子线程。因此，本程序的执行结果和顺序执行的代码类似，结果是确定的。

但是，当结主线程不加阻塞会发生什么呢？用指令`./thread_no_block`运行非阻塞版本的程序，结果如下。

    kevin_ling@Spear-of-Adun:~/Operating-System/ch4_homework$ ./thread_no_block
    35
    The average value is 0
    The minimum value is 0
    The maximum value is 0
    kevin_ling@Spear-of-Adun:~/Operating-System/ch4_homework$

可以看到，程序均输出 3 个全局变量的初始值 0。这是因为在非阻塞条件下，主线程还没等待子线程完成运算任务就将结果输出。此时的变量就仍然还是初始化的值了。

## 源程序

### 原版

```c
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

/*
    切记链接上 -lpthread 库
*/

int avg_num, max_num, min_num, len = 0; // 题目要求的3个全局变量
int pthread_kill(pthread_t thread, int sig);

/**
 * @brief Get the Avg object
 *
 * @param param
 * @return void*
 */
void *getAvg(void *param)
{
    int *seq = (int *)param, sum = 0;

    for (size_t i = 0; i < len; i++)
        sum += seq[i];

    avg_num = sum / len;

    pthread_exit(0);
}

/**
 * @brief Get the Max object
 *
 * @param param
 * @return void*
 */
void *getMax(void *param)
{ /* the thread */

    int *seq = (int *)param, max_temp = INT_MIN;

    for (size_t i = 0; i < len; i++)
        if (seq[i] > max_temp)
            max_temp = seq[i];

    max_num = max_temp;

    pthread_exit(0);
}

/**
 * @brief Get the Min object
 *
 * @param param
 * @return void*
 */
void *getMin(void *param)
{ /* the thread */

    int *seq = (int *)param, min_temp = INT_MAX;

    for (size_t i = 0; i < len; i++)
        if (seq[i] < min_temp)
            min_temp = seq[i];

    min_num = min_temp;

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int arr[1024], kill_rc;
    char c = '0';

    while (c != '\n')
    {
        scanf("%d", &arr[len]);
        c = getchar();
        len++;
    }

    pthread_t tid1, tid2, tid3;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 线程1
    pthread_create(&tid1, &attr, getAvg, arr);
    pthread_join(tid1, NULL);
    /*
        pthread_kill()函数用于判断线程是否结束。
        这么做的原因是，题目要求必须在子线程结束后输出结果。
        尽管pthread_join()作用就是以阻塞的方式等待thread指定的线程结束，
        但是使用pthread_kill()能增加程序容错性和鲁棒性。
    */
    kill_rc = pthread_kill(tid1, 0);
    if (kill_rc == ESRCH)
        printf("The average value is %d\n", avg_num);
    else if (kill_rc == EINVAL)
        printf("signal is invalid\n");
    else
        printf("the thread 1 is still alive\n");

    // 线程2
    pthread_create(&tid2, &attr, getMin, arr);
    pthread_join(tid2, NULL);
    kill_rc = pthread_kill(tid2, 0);
    if (kill_rc == ESRCH)
        printf("The minimum value is %d\n", min_num);
    else if (kill_rc == EINVAL)
        printf("signal is invalid\n");
    else
        printf("the thread 2 is still alive\n");

    // 线程3
    pthread_create(&tid3, &attr, getMax, arr);
    pthread_join(tid3, NULL);
    kill_rc = pthread_kill(tid3, 0);
    if (kill_rc == ESRCH)
        printf("The maximum value is %d\n", max_num);
    else if (kill_rc == EINVAL)
        printf("signal is invalid\n");
    else
        printf("the thread 3 is still alive\n");
}
```

### 非阻塞版

```c
#include <errno.h>
#include <limits.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>

/*
    切记链接上 -lpthread 库
*/

int avg_num, max_num, min_num, len = 0; // 题目要求的3个全局变量
int pthread_kill(pthread_t thread, int sig);

/**
 * @brief Get the Avg object
 *
 * @param param
 * @return void*
 */
void *getAvg(void *param)
{
    int *seq = (int *)param, sum = 0;

    for (size_t i = 0; i < len; i++)
        sum += seq[i];

    avg_num = sum / len;

    pthread_exit(0);
}

/**
 * @brief Get the Max object
 *
 * @param param
 * @return void*
 */
void *getMax(void *param)
{ /* the thread */

    int *seq = (int *)param, max_temp = INT_MIN;

    for (size_t i = 0; i < len; i++)
        if (seq[i] > max_temp)
            max_temp = seq[i];

    max_num = max_temp;

    pthread_exit(0);
}

/**
 * @brief Get the Min object
 *
 * @param param
 * @return void*
 */
void *getMin(void *param)
{ /* the thread */

    int *seq = (int *)param, min_temp = INT_MAX;

    for (size_t i = 0; i < len; i++)
        if (seq[i] < min_temp)
            min_temp = seq[i];

    min_num = min_temp;

    pthread_exit(0);
}

int main(int argc, char *argv[])
{
    int arr[1024], kill_rc;
    char c = '0';

    while (c != '\n')
    {
        scanf("%d", &arr[len]);
        c = getchar();
        len++;
    }

    pthread_t tid1, tid2, tid3;
    pthread_attr_t attr;
    pthread_attr_init(&attr);

    // 线程1
    pthread_create(&tid1, &attr, getAvg, arr);
    printf("The average value is %d\n", avg_num);

    // 线程2
    pthread_create(&tid2, &attr, getMin, arr);
    printf("The minimum value is %d\n", min_num);

    // 线程3
    pthread_create(&tid3, &attr, getMax, arr);
    printf("The maximum value is %d\n", max_num);
}
```
