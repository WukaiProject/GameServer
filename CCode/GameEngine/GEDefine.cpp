/*
游戏引擎的宏定义执行相关
*/
#include <iostream>
#include "GEDefine.h"

void GEAssertHelp( const char* _file, int _line, const char* _exp )
{
	std::cout<<_file<<" line:"<<_line<<" exp: "<<_exp<<std::endl;
}

void GECrashHelp( )
{
	int zero = 0;
	zero = 1 / zero;
}

