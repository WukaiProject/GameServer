/*我是UTF8无签名编码*/
#include "GENetRecvBuf.h"
#include "GEIO.h"

GENetRecvBuf::GENetRecvBuf(GE::Uint16 uBlockSize, GE::Uint16 uBlockNum)
	: m_uBlockNum(uBlockNum)
{
	this->m_pReadBuf = new GENetBuf(uBlockSize);
	this->m_pWriteBuf = new GENetBuf(uBlockSize);
}


GENetRecvBuf::~GENetRecvBuf(void)
{
	GE_SAFE_DELETE(m_pReadBuf);
	GE_SAFE_DELETE(m_pWriteBuf);
	while(!m_BufQueue.empty())
	{
		GE_SAFE_DELETE(m_BufQueue.front());
		m_BufQueue.pop();
	}
}

bool GENetRecvBuf::WriteMsg( MsgBase* pMsg )
{
	// 断言下长度
#ifdef WIN
	GE_ERROR(pMsg->Size() % 4 == 0);
#endif
	// 要写入的字节数不能超过block的最大大小
	if (this->m_pWriteBuf->MaxSize() < pMsg->Size())
	{
		GE_EXC<<"recv a long msg("<<pMsg->Type()<<", "<<pMsg->Size()<<") rather than block size("<<this->m_pWriteBuf->MaxSize()<<")."<<GE_END;
		return false;
	}
	// 字节太长
	if (this->m_pWriteBuf->CanWriteSize() < pMsg->Size())
	{
		// 并且此时扩容也不行了，则返回失败
		if (this->m_BufQueue.size() > this->m_uBlockNum)
		{
			return false;
		}
		this->m_BufQueue.push(this->m_pWriteBuf);
		this->m_pWriteBuf = new GENetBuf(this->m_pWriteBuf->MaxSize());
	}
	/***********************************************/
	/* 注意此时，可以保证写buf有足够的空间写入字节 */
	/***********************************************/
	// 将字节写入写buf
	this->m_pWriteBuf->WriteBytes_us(pMsg, pMsg->Size());
	this->m_pWriteBuf->MoveWriteFence_us(pMsg->Size());
	// 写入成功
	return true;
}

bool GENetRecvBuf::ReadMsgFromReadBuf( MsgBase** pMsg )
{
	// 当前读buf中没消息了
	if (0 == this->m_pReadBuf->CanReadSize())
	{
		return false;
	}
	// 从其中读取一条消息
	MsgBase* _pMsg = static_cast<MsgBase*>(this->m_pReadBuf->ReadFence_us());
	GE_ERROR(_pMsg->Size() <= m_pReadBuf->CanReadSize());
	this->m_pReadBuf->MoveReadFence_us(_pMsg->Size());
	*pMsg = _pMsg;
	return true;
}

bool GENetRecvBuf::MoveToNextReadBuf()
{
	// 断言下
#ifdef WIN
	GE_ERROR(0 == this->m_pReadBuf->CanReadSize());
#endif
	// 如果扩容列表为空，尝试直接交换写buf和读buf
	if (this->m_BufQueue.empty())
	{
		// 如果写buf里面有数据，则真正的交换两个buf
		if (this->m_pWriteBuf->CanReadSize())
		{
			std::swap(m_pReadBuf, m_pWriteBuf);
			// 初始化写buf
			this->m_pWriteBuf->Reset();
		}
		// 否则读取消息失败
		else
		{
			return false;
		}
	}
	// 扩容列表不为空删除当前的读buf，从扩容列表中拿出头部的buf当做读buf
	else
	{
		delete this->m_pReadBuf;
		this->m_pReadBuf = this->m_BufQueue.front();
		this->m_BufQueue.pop();
	}
	return true;
}

