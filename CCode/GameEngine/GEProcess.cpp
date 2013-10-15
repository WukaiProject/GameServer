/*我是UTF8无签名编码*/
#include <stdio.h>
#include <boost/lexical_cast.hpp>
#include "GEProcess.h"
#include "GEIO.h"

GEProcess::GEProcess(void)
{
}

GEProcess::~GEProcess(void)
{
}

void GEProcess::Main( int argc, char *argv[] )
{
	// 先进行写系统的检查
	this->SysCheck();
	// 一般情况下必须在启动进程的时候给个进程ID
	if (argc > 3)
	{
		m_ProcessType = argv[1];
		m_ProcessID = boost::lexical_cast<GE::Uint16>(argv[2]);
		m_ListenPort = boost::lexical_cast<GE::Uint16>(argv[3]);
	}
#ifdef WIN
	// 让用户输入一个非0的进程ID
	if(m_ProcessType.empty())
	{
		GE_OUT<<"input process group and id(not 0): ";
		GE_IN>>m_ProcessType>>m_ProcessID>>m_ListenPort;
	}
	// 注意清空输入缓冲区
	fflush(stdin);
#endif
}

bool GEProcess::IsWin()
{
#ifdef WIN
	return true;
#else
	return false;
#endif
}

bool GEProcess::IsLinux()
{
#ifdef LINUX
	return true;
#else
	return false;
#endif
}

bool GEProcess::IsDebug()
{
#ifdef _DEBUG
	return true;
#else
	return false;
#endif
}

bool GEProcess::IsRelease()
{
#ifdef _DEBUG
	return false;
#else
	return true;
#endif
}

void GEProcess::SysCheck()
{
	/*
	这里确保系统是小端结构
	*/
	GE_ERROR(sizeof(GE::B4) == 4);
	GE_ERROR(sizeof(GE::B8) == 8);
	GE_ERROR(sizeof(long) >= 4);
	GE::B4 b4;
	b4.UI16_0() = 1;
	GE_ERROR(b4 == 1);
	GE::B8 b8(5,0);
	GE_ERROR(b8 == 5);
}

