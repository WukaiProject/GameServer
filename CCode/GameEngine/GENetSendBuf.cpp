/*我是UTF8无签名编码*/
#include "GENetSendBuf.h"
#include "GEIO.h"


GENetSendBuf::GENetSendBuf(GE::Uint16 uBlockSize, GE::Uint16 uBlockNum)
	: m_bIsHoldBlock(false)
	, m_uBlockNum(uBlockNum)
{
	this->m_pReadBuf = new GENetBuf(uBlockSize);
	this->m_pWriteBuf = new GENetBuf(uBlockSize);
}


GENetSendBuf::~GENetSendBuf(void)
{
	GE_SAFE_DELETE(m_pReadBuf);
	GE_SAFE_DELETE(m_pWriteBuf);
	while(!m_BufQueue.empty())
	{
		GE_SAFE_DELETE(m_BufQueue.front());
		this->m_BufQueue.pop();
	}
}

bool GENetSendBuf::WriteBytes( const void* pHead, GE::Uint16 uSize )
{
	// 字节太长
	if (this->m_pWriteBuf->CanWriteSize() < uSize)
	{
		// 并且此时扩容也不行了，则返回失败
		if (this->m_BufQueue.size() > this->m_uBlockNum)
		{
			return false;
		}
		else
		{
			// 将可写的字节写入buf
			GE::Uint16 uL = this->m_pWriteBuf->CanWriteSize();
			this->m_pWriteBuf->WriteBytes_us(pHead, uL);
			this->m_pWriteBuf->MoveWriteFence_us(uL);
			// 修正要写入字节的头指针和长度
			pHead = (const char*)pHead + uL;
			uSize = uSize - uL;
			// 将当前的写buf加入扩容列表尾部，并新建一个写buf
			this->m_BufQueue.push(this->m_pWriteBuf);
			this->m_pWriteBuf = new GENetBuf(this->m_pWriteBuf->MaxSize());
			// 再次尝试写入
			return this->WriteBytes(pHead, uSize);
		}
	}
	// 写buf可以容纳字节数，直接写入之
	else
	{
		this->m_pWriteBuf->WriteBytes_us(pHead, uSize);
		this->m_pWriteBuf->MoveWriteFence_us(uSize);
		return true;
	}
}

bool GENetSendBuf::HoldBlock( void** pHead, GE::Uint16& uSize )
{
	// 如果已经被hold住了，直接返回之
	if (this->m_bIsHoldBlock)
	{
		return false;
	}
	// 扩容列表为空，尝试直接交换写buf和读buf
	if (this->m_BufQueue.empty())
	{
		// 如果写buf里面有数据，则真正的交换两个buf
		if (this->m_pWriteBuf->CanReadSize())
		{
			std::swap(m_pReadBuf, m_pWriteBuf);
			// 初始化写buf
			this->m_pWriteBuf->Reset();
		}
		// 否则肯定hold失败
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
	// 此时读buf必定是OK的
	// 将读buf带出去，并标识为已经hold
	*pHead = this->m_pReadBuf->HeadPtr();
	uSize = this->m_pReadBuf->CanReadSize();
	this->m_bIsHoldBlock = true;
	return true;
}

void GENetSendBuf::ReleaseBlock()
{
	// 断言此时有Block被外部hold住
	GE_ERROR(true == m_bIsHoldBlock);
	// 初始化buf
	this->m_pReadBuf->Reset();
	// 置标志位
	this->m_bIsHoldBlock = false;
}

bool GENetSendBuf::IsEmpty()
{
	return this->m_BufQueue.empty() && this->m_pReadBuf->CanReadSize() == 0 && this->m_pWriteBuf->CanReadSize() == 0;
}

