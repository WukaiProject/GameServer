/************************************************************************
进程约束
约束1： 每个进程都有一个唯一的ID
************************************************************************/
#pragma once
#include <string>
#include "GEInteger.h"
#include "GESingleton.h"

class GEProcess
	: public GESingleton<GEProcess>
{
	GE_DISABLE_BOJ_CPY(GEProcess);
public:
	GEProcess(void);
	~GEProcess(void);

public:
	void			Main(int argc, char *argv[]);				//初始化
	std::string&	ProcessType() {return m_ProcessType;}		//进程分组
	GE::Uint16		ProcessID() {return m_ProcessID;}			//进程ID
	GE::Uint16		LisPort() {return m_ListenPort;}			//监听端口
	bool			IsWin();									//是否是Window环境
	bool			IsLinux();									//是否是Linux环境
	bool			IsDebug();									//是否是Debug环境
	bool			IsRelease();								//是否是Release环境
	
private:
	void			SysCheck();									//系统检测

private:
	std::string		m_ProcessType;
	GE::Uint16		m_ProcessID;
	GE::Uint16		m_ListenPort;
};

