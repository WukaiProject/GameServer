/************************************************************************
网络消息发送缓存区
************************************************************************/
#pragma once
#include <queue>
#include "GEInteger.h"
#include "GENetBuf.h"

class GENetSendBuf
{
	GE_DISABLE_BOJ_CPY(GENetSendBuf);
	typedef std::queue<GENetBuf*>	BufQueue;
public:
	GENetSendBuf(GE::Uint16 uBlockSize, GE::Uint16 uBlockNum);
	~GENetSendBuf(void);

public:
	bool			WriteBytes(const void* pHead, GE::Uint16 uSize);		//将一段数据写入发送消息缓存区
	bool			HoldBlock(void** pHead, GE::Uint16& uSize);				//从发送消息缓冲区中hold出一块要发送的数据
	void			ReleaseBlock();											//上次hold的数据发送完毕，归还这段数据的控制权限
	bool			IsEmpty();												//Buf是否是空的

private:
	GENetBuf*		m_pReadBuf;
	GENetBuf*		m_pWriteBuf;
	BufQueue		m_BufQueue;
	bool			m_bIsHoldBlock;
	GE::Uint16		m_uBlockNum;
};

