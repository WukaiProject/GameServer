/************************************************************************
64位的全局ID分配器
分配器是按照时间进行分配的，在时间正确的情况下可以分配全球唯一ID
进程重启是不会影响ID的分配的，故其分配出来的ID可以保存
************************************************************************/
#pragma once
#include "GEInteger.h"
#include "GESingleton.h"

class GEGUID64
	: public GESingleton<GEGUID64>
{
public:
	GEGUID64(void);
	~GEGUID64(void);

public:
	void				AllotFromNow();						//开始分配
	void				AllotPerMinute();					//每分钟加大分配上限
	void				AllotGUID(GE::Uint64& b8);			//分配一个全球ID
	GE::Uint64			AllotGUID();						//分配一个全球ID

private:
	GE::Uint64			m_uBaseID;
	GE::Uint64			m_uMaxID;
};

