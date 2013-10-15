/************************************************************************
游戏引擎的宏定义相关
************************************************************************/
#pragma once
#include <assert.h>
#include <boost/static_assert.hpp>

// 辅助函数
void GEAssertHelp(const char* _file, int _line, const char* _exp);
void GECrashHelp();

// 禁止拷贝
#define GE_DISABLE_BOJ_CPY(cname)	\
	cname(const cname&);	\
	const cname& operator=(const cname&);

// 安全删除
#define GE_SAFE_DELETE(_ptr)	\
	if((_ptr)) { \
	delete (_ptr);	\
	(_ptr) = 0;	\
	};

// 警告、错误、断言和静态断言
#define GE_WARN(_exp) if(!(_exp)){GEAssertHelp(__FILE__, __LINE__, #_exp);}
#define GE_ERROR(_exp) if(!(_exp)){GEAssertHelp(__FILE__, __LINE__, #_exp);GECrashHelp();}
#ifdef _DEBUG
#define GE_ASSERT GE_ERROR
#else
#define GE_ASSERT GE_WARN
#endif
#define GE_STATIC_ASSERT BOOST_STATIC_ASSERT

// 最大值，最小值
#define GE_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define GE_MIN(a, b) (((a) < (b)) ? (a) : (b))

