/************************************************************************
系统的文件日志
************************************************************************/
#pragma once
#include <iostream>
#include <fstream>
#ifdef WIN
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif
#include "GEDateTime.h"

// 特殊标记定义
#define EXCEPT_FLAG "GE_EXC"
#define TRACEBACK_FLAG "Traceback"

// 从定向输出和错误流
void RedirectOutAndErrStream(const char* sFilePath);

#ifdef WIN
inline std::ostream& Blue(std::ostream &s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
		|FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	return s;
} 

inline std::ostream& Red(std::ostream &s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED|FOREGROUND_INTENSITY);
	return s;
} 

inline std::ostream& Green(std::ostream &s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN|FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& Yellow(std::ostream &s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN|FOREGROUND_RED|FOREGROUND_INTENSITY);
	return s;
}

inline std::ostream& White(std::ostream &s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED|FOREGROUND_GREEN|FOREGROUND_BLUE);
	return s;
}
#endif

/************************************************************************
GE_IN   是对标准输入流的一个统一的宏
GE_OUT  是对标准输出流的一个统一的宏，标准输出流可被被重定向到文件
GE_EXC  是带特殊标准的标准输出流的一个统一的宏，可做关键字扫描日志文件
GE_END  是对标准结束符的一个统一的宏
************************************************************************/
#ifdef WIN
#define GE_IN  std::cin
#define GE_OUT std::cout<<White
#define GE_ERR std::cerr
#define GE_EXC std::cout<<Yellow<<GEDateTime::Instance()->Hour()<<":"<<GEDateTime::Instance()->Minute()<<":"<<GEDateTime::Instance()->Second()<<" "
#define GE_END White<<std::endl
#elif LINUX
#define GE_IN  std::cin
#define GE_OUT std::cout
#define GE_ERR std::cerr
#define GE_EXC std::cout<<EXCEPT_FLAG<<" "<<GEDateTime::Instance()->Year()<<"-"<<GEDateTime::Instance()->Month()<<"-"<<GEDateTime::Instance()->Day()<<" "<<GEDateTime::Instance()->Hour()<<":"<<GEDateTime::Instance()->Minute()<<":"<<GEDateTime::Instance()->Second()<<" "
#define GE_END std::endl
#endif

