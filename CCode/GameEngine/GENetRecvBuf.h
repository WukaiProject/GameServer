/************************************************************************
一个网络连接的接收消息缓存区
************************************************************************/
#pragma once
#include <queue>
#include "GEInteger.h"
#include "GENetBuf.h"
#include "GENetMessage.h"

class GENetRecvBuf
{
	GE_DISABLE_BOJ_CPY(GENetRecvBuf);
	typedef std::queue<GENetBuf*>	BufQueue;
public:
	GENetRecvBuf(GE::Uint16 uBlockSize, GE::Uint16 uBlockNum);
	~GENetRecvBuf(void);

public:
	bool			WriteMsg(MsgBase* pMsg);				//向接收消息缓冲区中写入一条消息
	bool			ReadMsgFromReadBuf(MsgBase** pMsg);		//从接收消息缓冲区的读buf中读取一条消息
	bool			MoveToNextReadBuf();					//获取下一个读buf

private:
	GENetBuf*		m_pReadBuf;
	GENetBuf*		m_pWriteBuf;
	GE::Uint16		m_uBlockNum;
	BufQueue		m_BufQueue;
};

