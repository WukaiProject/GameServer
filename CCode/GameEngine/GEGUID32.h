/************************************************************************
32位的全局GUID分配器
需要一个8位的基础ID，来区别全局的唯一性，总共可分配16777216
进程重启将从头再分配，故分配出来的ID不可保存
************************************************************************/
#pragma once
#include "GEInteger.h"

class GEGUID32
{
public:
	GEGUID32(void);
	GEGUID32(GE::Uint8 uBase);
	~GEGUID32(void);

public:
	void				SetBaseID(GE::Uint8 uBase);			//设置高8位
	bool				AllotGUID(GE::Uint32& uID);			//分配一个ID

private:
	GE::Uint32			m_uAllotID;
	GE::Uint32			m_uMaxID;
};

