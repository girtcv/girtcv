#pragma once
//#ifndef _DEBUG_H_
//#define _DEBUG_H_
///*************************调试输出宏**************************************///
#if defined _DBG
#define _DEBUGGING_
#endif

#ifdef _DEBUGGING_
#ifdef __cplusplus
#include <cstdio>
#else
#include <stdio.h>
#endif
#endif

///@brief 一般放于函数末尾, 判断CPU是否执行函数成功
#ifdef _DEBUGGING_
#define TAG() printf("%s: %d rows %s succeed\n", __FILE__, __LINE__, __func__)
#else
#define TAG()
#endif

///@brief 打印变量的值
///@param name 变量的名称
///@param value 要查看的变量
#ifdef _DEBUGGING_
#define MYDBG(name, value) printf("%s:%x\n", name, value)
#else
#define MYDBG(name, value)
#endif

///@brief 功能等同与printf 为了兼容c89标准
///@brief SHOW(("num:%d", 0)); <=> printf("num:%d", 0);
#ifdef _DEBUGGING_
#define SHOW(args) printf args
#else
#define SHOW(args)
#endif

///@brief debug output c99标准之后才支持
///@brief COUT("num:%d", 0); <=> printf("num:%d", 0);
#ifdef _DEBUGGING_
#define COUT(...) printf(__VA_ARGS__)
#else
#define COUT(...)
#endif

//#endif