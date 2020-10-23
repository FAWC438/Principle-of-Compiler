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

_编译器_：GCC-9 on Linux

_词法分析器_：flex on Linux

---

## 程序设计说明

### 文件说明

    2018211314-凌国瀚-2018213344
    ├── CPP
    |   ├── analysis
    |   ├── analysis.cpp
    |   ├── in.c
    |   ├── out.txt
    |   └── table.txt
    ├── LEX
    |   ├── a.out
    |   ├── lex_source.l
    |   ├── lex.yy.c
    |   ├── result.txt
    |   └── test1.c
    └── README.pdf

文件树如上所示，其中`CPP`文件夹存放C++版本的词法分析程序，。

`thread.c`为本作业源代码，`thread`为可执行文件。

在作业要求外，额外加入了非阻塞版本的代码`thread_no_block.c`及其可执行文件`thread_no_block`，以作为对照样例说明程序运行结果

`README.md`为本说明文档，`README.pdf`是本文档的 pdf 版本，以便于在没有 Markdown 浏览条件时阅读本文档。

欲运行本程序，请参照[运行说明](#运行说明)。

### 运行说明

欲编译本程序并生成可执行文件，请在**源程序所在目录**的 Linux bash 中输入如下指令：

    gcc thread.c -o thread -lpthread

即可生成可执行文件`thread`。

> **温馨提示**
> 由于本程序调用 pthread 线程库，因此必须添加参数`-lpthread` 以实现库的动态链接

随后，直接使用以下指令即可运行可执行文件：

    ./thread

非阻塞版本程序的运行方式同上。

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
